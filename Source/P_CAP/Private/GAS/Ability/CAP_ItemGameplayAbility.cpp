// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_ItemGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_ItemDataAsset.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Items/Item/CAP_ItemInstance.h"

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

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	
	for (int32 i=0 ; i<ItemData->ItemSkills.Num() ; ++i)
	{
		const FItemSkillData& SkillData = ItemData->ItemSkills[i];
		if (SkillData.ActiveType == EItemSkillActiveType::Trigger || SkillData.TriggerEventTag.MatchesTagExact(Payload.EventTag))
		{
			if (ASC && SkillData.CooldownTag.IsValid() && ASC->HasMatchingGameplayTag(SkillData.CooldownTag))
				continue;
			
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
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC)
		return;
	if (SkillData.CooldownTag.IsValid() && SourceASC->HasMatchingGameplayTag(SkillData.CooldownTag))
		return;

	UCAP_AbilitySystemComponent* CAP_ASC = Cast<UCAP_AbilitySystemComponent>(SourceASC);
	if (!CAP_ASC && !CAP_ASC->GetGenerics())
		return;

	UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(GetCurrentAbilitySpec()->SourceObject);

	// 쿨타임 적용
	if (CAP_ASC->GetGenerics()->GetCooldownEffect())
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CAP_ASC->GetGenerics()->GetCooldownEffect(), GetAbilityLevel());
		SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Cooldown"), SkillData.Cooldown);
		if (SkillData.CooldownTag.IsValid())
		{
			SpecHandle.Data->DynamicGrantedTags.AddTag(SkillData.CooldownTag);
		}
		ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle);
	}
	
	// 액션 별 분기
	for (const FItemEffectPayload& Effect : SkillData.ItemEffectPayloads)
	{
		// 확률로 실행되는 부분
		if (FMath::RandRange(0.f,100.f)>Effect.TriggerChance)
			continue;

		// 특수 로직
		if (Effect.ExecutionType == EItemExecutionType::Destroy_Item)
		{
			UE_LOG(LogTemp, Warning, TEXT("아이템이 파괴되었습니다"));
			if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetAvatarActorFromActorInfo()))
				Player->GetInventoryComponent()->RemoveItem(ItemInst);
			continue;
		}
		else if (Effect.ExecutionType == EItemExecutionType::Upgrade_Item)
		{
			UE_LOG(LogTemp, Warning, TEXT("아이템이 업그레이드 되었습니다"));
			continue;
		}
		else if (Effect.ExecutionType == EItemExecutionType::Custom_Ability)
		{
			if (Effect.CustomAbilityClass)
			{
				FGameplayAbilitySpec Spec(Effect.CustomAbilityClass, 1, INDEX_NONE, ItemInst);
				SourceASC->TryActivateAbility(SourceASC->GiveAbility(Spec));
			}
			continue;
		}

		TSubclassOf<UGameplayEffect> MasterGE = CAP_ASC->GetGenerics()->GetItemMasterGE(Effect.ExecutionType);
		if (!MasterGE)
		{
			UE_LOG(LogTemp,Warning, TEXT("Master GE 못찾았음"));
			continue;
		}

		TArray<UAbilitySystemComponent*> TargetASCs;
		// 나에게 적용할지, 적에게 적용할지
		if (Effect.ExecutionType == EItemExecutionType::Buff_Self)
		{	// 나에게 적용하는 경우, 나의 ASC 가져오기
			TargetASCs.Add(SourceASC);
		}
		else if (Payload.TargetData.Num() >0)
		{	// 적에게 적용하는 경우, 타격된 대상의 ASC 모두 가져오기
			UE_LOG(LogTemp,Warning, TEXT("Payload.TargetData.Num() = %d"), Payload.TargetData.Num());
			for (auto& Data : Payload.TargetData.Data)
			{
				if (Data.IsValid())
				{
					for (auto TargetActor : Data->GetActors())
					{
						UE_LOG(LogTemp,Warning, TEXT("맞은 대상 = %s"), *TargetActor->GetName());
						if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor.Get()))
							TargetASCs.AddUnique(TargetASC);
					}
				}
			}
		}
		
		// 데미지 수치 조정 ( 기본 데미지 + 스탯*계수 )
		float FinalMagnitude = Effect.BaseValue;
		if (Effect.ScaleAttribute.IsValid())
		{
			float StatValue = SourceASC->GetNumericAttribute(Effect.ScaleAttribute);
			FinalMagnitude += (StatValue * Effect.Magnitude);
		}

		// 각 대상에게 독립적 스택 검사 & 적용 (A몬스터는 방깍 스택 5 / B몬스터는 방깍 스택 2 - 서로 독립적으로) 
		for (UAbilitySystemComponent* DestASC : TargetASCs)
		{
			int32 CurrentStacks =0;
			// 갱신하기 위해 지울 기존 핸들 Ary
			TArray<FActiveGameplayEffectHandle> HandlesToRemove;
			
			if (Effect.DynamicTag.IsValid())
			{
				FGameplayEffectQuery Query;
				Query.EffectTagQuery = FGameplayTagQuery::MakeQuery_MatchAnyTags(Effect.DynamicTag.GetSingleTagContainer());
				TArray<FActiveGameplayEffectHandle> ActiveHandles = DestASC->GetActiveEffects(Query);

				for (const FActiveGameplayEffectHandle& Handle : ActiveHandles)
				{
					const FActiveGameplayEffect* ActiveGE = DestASC->GetActiveGameplayEffect(Handle);
					if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == ItemInst)
					{
						CurrentStacks += ActiveGE->Spec.GetStackCount();
						// 고유의 아이템이 건 버프면 지울 Ary에 삽입
						HandlesToRemove.Add(Handle);
					}
				}
			}

			// 목표 스택 계산
			int32 TargetStackCount = CurrentStacks +1;
			if (Effect.MaxStackCount >0)
				TargetStackCount = FMath::Min(TargetStackCount, Effect.MaxStackCount);

			// 기존 스택들 삭제
			for (const FActiveGameplayEffectHandle& Handle : HandlesToRemove)
				DestASC->RemoveActiveGameplayEffect(Handle);

			for (int32 i=0 ; i<TargetStackCount ; ++i)
			{
				FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
				Context.AddSourceObject(ItemInst);

				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MasterGE,GetAbilityLevel(), Context);
				if (SpecHandle.IsValid())
				{
					FGameplayTag ValueTag = Effect.TargetStatTag.IsValid() ? Effect.TargetStatTag : FGameplayTag::RequestGameplayTag("Data.Value");
					SpecHandle.Data->SetSetByCallerMagnitude(ValueTag, FinalMagnitude);

					if (Effect.Duration >0.f)
						SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Duration"), Effect.Duration);
					if (Effect.DynamicTag.IsValid())
						SpecHandle.Data->DynamicGrantedTags.AddTag(Effect.DynamicTag);

					// 나머지 값 0으로 초기화하여 에러 로그 방지
					if (const UGameplayEffect* DefaultGE = MasterGE->GetDefaultObject<UGameplayEffect>())
					{
						for (const FGameplayModifierInfo& ModInfo : DefaultGE->Modifiers)
						{
							if (ModInfo.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
							{
								FGameplayTag CallerTag = ModInfo.ModifierMagnitude.GetSetByCallerFloat().DataTag;
								
								if (CallerTag.IsValid() && CallerTag != ValueTag && CallerTag != FGameplayTag::RequestGameplayTag("Data.Duration"))
								{
									SpecHandle.Data->SetSetByCallerMagnitude(CallerTag, 0.f);
								}
							}
						}
					}
					
					DestASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
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
{
	if (GetCurrentAbilitySpec() && GetCurrentAbilitySpec()->SourceObject.IsValid())
	{
		if (UCAP_ItemDataAsset* ItemDA = Cast<UCAP_ItemDataAsset>(GetCurrentAbilitySpec()->SourceObject.Get()))
			return ItemDA;
		if (UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(GetCurrentAbilitySpec()->SourceObject.Get()))
			return Cast<UCAP_ItemDataAsset>(ItemInst->GetItemDA());
	}
	return nullptr;
}
