// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_ItemGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_ItemDataAsset.h"
#include "GAS/CAP_AbilitySystemComponent.h"

UCAP_ItemGameplayAbility::UCAP_ItemGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UCAP_ItemGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,const FGameplayEventData* TriggerEventData)
{
	// 아이템 착용 시점에 1회 호출
	const UCAP_ItemDataAsset* ItemData = GetItemData();
	if (!ItemData)
	{
		K2_EndAbility();
		return;
	}
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UE_LOG(LogTemp,Warning,TEXT("아이템 장착됨"));
	
	for (int32 i=0 ; i<ItemData->ItemSkills.Num() ; ++i)
	{
		const FItemSkillData& SkillData = ItemData->ItemSkills[i];
		TriggerCounts.Add(i,0);

		// 각 타입에 맞는 로직을 1회만 호출 - 반복은 내부에서 WaitGameplayEvent Task와 Timer로 조절
		if (SkillData.ActiveType == EItemSkillActiveType::Trigger)
		{
			HandleTriggerLogic(SkillData,i);
		}
		else
		{
			HandleAutoLogic(SkillData,i);
		}
	}
}

void UCAP_ItemGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	for (auto& TimerPair : AutoCastTimer)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerPair.Value);
	}
	AutoCastTimer.Empty();
	TriggerCounts.Empty();
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UCAP_ItemGameplayAbility::HandleTriggerLogic(const struct FItemSkillData& SkillData, int32 SkillIndex)
{
	UE_LOG(LogTemp,Warning,TEXT("트리거 아이템 실행"));
	
	UAbilityTask_WaitGameplayEvent* WaitEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, SkillData.TriggerEventTag);
	WaitEvent->EventReceived.AddDynamic(this, &UCAP_ItemGameplayAbility::OnTriggerEventReceived);
	WaitEvent->ReadyForActivation();
}

void UCAP_ItemGameplayAbility::HandleAutoLogic(const struct FItemSkillData& SkillData, int32 SkillIndex)
{
	UE_LOG(LogTemp,Warning,TEXT("자동 발동 아이템 장착됨"));
	
	float LoopTime = FMath::Max(0.1f, SkillData.Cooldown);
	FTimerHandle Timer;
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this, FName("ExecuteAutoCast"), SkillIndex);
	GetWorld()->GetTimerManager().SetTimer(Timer, TimerDel, LoopTime, true);
	AutoCastTimer.Add(SkillIndex, Timer);
}

void UCAP_ItemGameplayAbility::OnTriggerEventReceived(FGameplayEventData Payload)
{
	const UCAP_ItemDataAsset* ItemData = GetItemData();
	if (!ItemData)
		return;
	for (int32 i=0 ; i<ItemData->ItemSkills.Num() ; ++i)
	{
		const FItemSkillData& SkillData = ItemData->ItemSkills[i];

		if (SkillData.ActiveType == EItemSkillActiveType::Trigger && SkillData.TriggerEventTag.MatchesTagExact(Payload.EventTag))
		{
			UE_LOG(LogTemp,Warning,TEXT("트리거 발동됨 1 증가"));
			
			TriggerCounts[i]++;
			if (TriggerCounts[i] >= SkillData.RequiredTriggerCount)
			{
				TriggerCounts[i] = 0;
				ExecuteEffect(SkillData, Payload);
			}
		}
	}
}

void UCAP_ItemGameplayAbility::ExecuteAutoCast(int32 SkillIndex)
{
	const UCAP_ItemDataAsset* ItemData = GetItemData();
	if (ItemData && ItemData->ItemSkills.IsValidIndex(SkillIndex))
	{
		const FItemSkillData& SkillData = ItemData->ItemSkills[SkillIndex];
		ExecuteEffect(SkillData, FGameplayEventData());
	}
}

void UCAP_ItemGameplayAbility::ExecuteEffect(const struct FItemSkillData& SkillData, FGameplayEventData Payload)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
		return;
	if (SkillData.CooldownTag.IsValid() && ASC->HasMatchingGameplayTag(SkillData.CooldownTag))
		return;

	// 쿨타임 적용
	UCAP_AbilitySystemComponent* CAP_ASC = Cast<UCAP_AbilitySystemComponent>(ASC);
	if (CAP_ASC && CAP_ASC->GetGenerics() && CAP_ASC->GetGenerics()->GetCooldownEffectClass())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CAP_ASC->GetGenerics()->GetCooldownEffectClass(), GetAbilityLevel());
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Cooldown"), SkillData.Cooldown);
		if (SkillData.CooldownTag.IsValid())
		{
			SpecHandle.Data->DynamicGrantedTags.AddTag(SkillData.CooldownTag);
		}
		ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle);
	}

	for (const FItemEffectPayload& Effect : SkillData.ItemEffectPayloads)
	{
		if (!Effect.MasterGEClass)
			continue;
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(Effect.MasterGEClass, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			float FinalMagnitude = Effect.Magnitude;

			// 설정한 스탯값 존재 시 (도트데미지 설정)
			if (Effect.BaseAttribute.IsValid())
			{
				float StatValue = ASC->GetNumericAttribute(Effect.BaseAttribute);
				FinalMagnitude = StatValue * Effect.Magnitude;
			}
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Value"), FinalMagnitude);

			// 디버프 태그 설정
			if (Effect.DynamicTag.IsValid())
			{
				SpecHandle.Data->DynamicGrantedTags.AddTag(Effect.DynamicTag);
			}

			if (Payload.TargetData.Num() > 0)
			{
				ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle, Payload.TargetData);
			}
			else
			{
				ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle);
			}
		}
	}
	if (SkillData.SpawnActorClass)
	{
		FTransform SpawnTrans = GetAvatarActorFromActorInfo()->GetActorTransform();
		GetWorld()->SpawnActor<AActor>(SkillData.SpawnActorClass, SpawnTrans);
	}
}

const class UCAP_ItemDataAsset* UCAP_ItemGameplayAbility::GetItemData()
{	// GiveAbility를 할 때 SourceObject로 ItemData 넘겨줘야함
	if (GetCurrentAbilitySpec())
	{
		return Cast<UCAP_ItemDataAsset>(GetCurrentAbilitySpec()->SourceObject);
	}
	return nullptr;
}
