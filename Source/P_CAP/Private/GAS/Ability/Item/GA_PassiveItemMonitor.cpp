// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Item/GA_PassiveItemMonitor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/ItemBehavior/ItemBehavior_ConditionalPassive.h"

UGA_PassiveItemMonitor::UGA_PassiveItemMonitor()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
}

void UGA_PassiveItemMonitor::InitMonitorData(const struct FItemConditionData& InCondition,
	const struct FItemActionData& InAction)
{
}

void UGA_PassiveItemMonitor::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	
	// 1. SourceObject에서 아이템 데이터 가져오기
	UItemBehavior_ConditionalPassive* ItemBehavior = Cast<UItemBehavior_ConditionalPassive>(GetCurrentSourceObject());
	if (!ASC || !ItemBehavior)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ConditionData = ItemBehavior->ConditionData;
	ActionData = ItemBehavior->ActionData;

	// 2. 스탯 변화 감시기(Delegate) 부착
	ASC->GetGameplayAttributeValueChangeDelegate(ConditionData.MonitorAttribute).AddUObject(this, &UGA_PassiveItemMonitor::OnAttributeChanged);

	// 3. 최초 1회 즉시 평가 실행
	EvaluateCondition();
}

void UGA_PassiveItemMonitor::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(ConditionData.MonitorAttribute).RemoveAll(this);
		
		if (ActiveBuffHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(ActiveBuffHandle);
			ActiveBuffHandle.Invalidate();
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_PassiveItemMonitor::OnAttributeChanged(const struct FOnAttributeChangeData& ChangeData)
{
	EvaluateCondition();
}

void UGA_PassiveItemMonitor::EvaluateCondition()
{
	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	
	if (!ASC || !ASC->GetGenerics()) return;

	// 1. 비교할 수치 연산 (절대값이냐 비율이냐)
	float CurrentValue = ASC->GetNumericAttribute(ConditionData.MonitorAttribute);
	float CompareValue = CurrentValue;

	if (ConditionData.bIsPercentage)
	{
		float MaxValue = ASC->GetNumericAttribute(ConditionData.MaxAttribute);
		CompareValue = (MaxValue > 0.f) ? (CurrentValue / MaxValue) : 0.f;
	}

	// 2. 임계치 도달 여부 판별
	bool bIsMet = false;
	switch (ConditionData.Operator)
	{
		case ECompareOperator::Equal:				bIsMet = FMath::IsNearlyEqual(CompareValue, ConditionData.Threshold); break;
		case ECompareOperator::GreaterThan:			bIsMet = CompareValue > ConditionData.Threshold; break;
		case ECompareOperator::GreaterThanOrEqual:	bIsMet = CompareValue >= ConditionData.Threshold; break;
		case ECompareOperator::LessThan:			bIsMet = CompareValue < ConditionData.Threshold; break;
		case ECompareOperator::LessThanOrEqual:		bIsMet = CompareValue <= ConditionData.Threshold; break;
	}

	// 3. 상태 전환 및 마스터 버프(GE) 제어
	if (bIsMet && !bIsConditionMet)
	{
		// [조건 진입] 버프 켜기
		bIsConditionMet = true;
		TSubclassOf<UGameplayEffect> MasterBuffGE = ASC->GetGenerics()->GetStatGE(true,false);
		if (MasterBuffGE)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
			
			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(MasterBuffGE, 1.f, Context);
			if (SpecHandle.IsValid())
			{
				// TMap에 정의된 수치들을 SetByCaller로 모두 밀어넣음
				for (const auto& ModPair : ActionData.StatModifiers)
				{
					SpecHandle.Data->SetSetByCallerMagnitude(ModPair.Key, ModPair.Value);
				}
				ActiveBuffHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		// 이벤트 태그가 있다면 발송 (예: 파장 이펙트 생성기용)
		if (ActionData.TriggerEventTag.IsValid())
		{
			FGameplayEventData Payload;
			Payload.Instigator = GetAvatarActorFromActorInfo();
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActorFromActorInfo(), ActionData.TriggerEventTag, Payload);
		}
	}
	else if (!bIsMet && bIsConditionMet)
	{
		// [조건 이탈] 버프 끄기
		bIsConditionMet = false;
		if (ActiveBuffHandle.IsValid())
		{
			ASC->RemoveActiveGameplayEffect(ActiveBuffHandle);
			ActiveBuffHandle.Invalidate();
		}
	}
}
