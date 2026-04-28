// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/CAP_ItemGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_ItemDataAsset.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Items/Item/CAP_ItemInstance.h"

UCAP_ItemGameplayAbility::UCAP_ItemGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ItemEffectDurationTag = UCAP_AbilitySystemStatics::GetDataItemEffectDurationTag();
	DataCooldownTag = UCAP_AbilitySystemStatics::GetDataCooldownTag();
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
		SpecHandle.Data->SetSetByCallerMagnitude(DataCooldownTag, SkillData.Cooldown);
		if (SkillData.CooldownTag.IsValid())
		{
			SpecHandle.Data->DynamicGrantedTags.AddTag(SkillData.CooldownTag);
		}
		ApplyGameplayEffectSpecToOwner(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle);
	}

	// Payload에서 HitResult 꺼내오기
	const FHitResult* HitResult = nullptr;
	if (Payload.TargetData.Num() > 0)
		HitResult = Payload.TargetData.Get(0)->GetHitResult();
	
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

		TArray<UAbilitySystemComponent*> TargetASCs = GetTargetASCs(Effect,Payload,SourceASC);		

		// 타격 대상에 독립적 스택 검사 & 스택 업데이트
		for (UAbilitySystemComponent* TargetASC : TargetASCs)
		{
			int32 CurrentStacks =0;					// 현재 쌓은 스택 개수
			FActiveGameplayEffectHandle OldHandle;	// 버프를 통해 스탯을 올릴 때 사용된 GE - 스택이 바뀔 때 이전의 스택으로 받은 버프 제거하기 위해 
			float AppliedBonus=0.f;					// 일시적 버프로 상승한 스탯 값

			// 적용되던 버프 탐색 (OldHandle, 현재 스택, 적용된 보너스 스탯에 값 지정)
			FindExistingStack(TargetASC, Effect, ItemInst, OldHandle, CurrentStacks, AppliedBonus);

			// 영구적 버프로만 계산하여 적용시킬 값
			float FinalMagnitude = CalculateCleanMagnitude(SourceASC, Effect, AppliedBonus);
			int32 TargetStackCount = FMath::Min(CurrentStacks+1, Effect.MaxStackCount >0 ? Effect.MaxStackCount : 999);
			
			// 업데이트 이전의 스택 효과 삭제
			if (OldHandle.IsValid())
				TargetASC->RemoveActiveGameplayEffect(OldHandle);

			ApplyEffectToTarget(SourceASC, TargetASC, Effect, ItemInst, FinalMagnitude,TargetStackCount, HitResult, MasterGE);
		}
	}
	if (SkillData.SpawnActorClass)
	{
		FTransform SpawnTrans = GetAvatarActorFromActorInfo()->GetActorTransform();
		GetWorld()->SpawnActor<AActor>(SkillData.SpawnActorClass, SpawnTrans);
	}
}

TArray<UAbilitySystemComponent*> UCAP_ItemGameplayAbility::GetTargetASCs(const struct FItemEffectPayload& Effect,
	const FGameplayEventData& Payload, UAbilitySystemComponent* SourceASC) const
{
	TArray<UAbilitySystemComponent*> TargetASCs;

	// 플레이어에게 적용되는 아이템 - 플레이어의 ASC를 챙겨
	if (Effect.ExecutionType == EItemExecutionType::Buff_Self)
	{
		UE_LOG(LogTemp,Warning, TEXT("Buff Self 아이템. 플레이어의 ASC를 챙깁니다"));
		TargetASCs.Add(SourceASC);
	}
	// 타격된 대상에게 적용되는 아이템 - 타격 대상의 ASC를 챙겨
	else if (Payload.TargetData.Num() >0)
	{
		UE_LOG(LogTemp,Warning, TEXT("타격된 대상의 ASC를 챙깁니다"));
		for (auto& Data : Payload.TargetData.Data)
		{
			if (Data.IsValid())
			{
				for (auto TargetActor : Data->GetActors())
				{
					if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor.Get()))
					{
						UE_LOG(LogTemp,Warning, TEXT("이 ASC의 주인은 = %s"), *TargetActor->GetName());
						TargetASCs.Add(TargetASC);
					}
				}
			}
		}
	}
	return TargetASCs;
}

void UCAP_ItemGameplayAbility::FindExistingStack(UAbilitySystemComponent* TargetASC,
	const struct FItemEffectPayload& Effect, class UCAP_ItemInstance* ItemInst,
	FActiveGameplayEffectHandle& OutOldHandle, int32& OutCurrentStacks, float& OutAppliedBonus) const
{
	OutOldHandle = FActiveGameplayEffectHandle();
	OutCurrentStacks = 0;
	OutAppliedBonus = 0.f;

	if (!Effect.DynamicTag.IsValid())
		return;
	
	FGameplayEffectQuery Query;
	TArray<FActiveGameplayEffectHandle> AllHandles = TargetASC->GetActiveEffects(Query);
	for (const FActiveGameplayEffectHandle& Handle : AllHandles)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE)
		{
			bool bHasTag = ActiveGE->Spec.DynamicGrantedTags.HasTagExact(Effect.DynamicTag);
			bool bSameSource = (ActiveGE->Spec.GetEffectContext().GetSourceObject() == ItemInst);
			if (bHasTag && bSameSource)
			{
				FGameplayTag StackTag = FGameplayTag::RequestGameplayTag("Data.StackCount");
				OutCurrentStacks = FMath::RoundToInt(ActiveGE->Spec.GetSetByCallerMagnitude(StackTag, false, 1.f));
				OutOldHandle = Handle;
				
				if (Effect.ExecutionType == EItemExecutionType::Buff_Self)
				{
					FGameplayTag ValueTag = Effect.TargetStatTag.IsValid() ? Effect.TargetStatTag : FGameplayTag::RequestGameplayTag("Data.Value");
					OutAppliedBonus = ActiveGE->Spec.GetSetByCallerMagnitude(ValueTag, false, 0.f);
				}
				break;
			}
		}
	}
}

float UCAP_ItemGameplayAbility::CalculateCleanMagnitude(UAbilitySystemComponent* SourceASC,
	const struct FItemEffectPayload& Effect, float AppliedBonus) const
{
	float FinalMagnitude = Effect.BaseValue;
	if (Effect.ScaleAttribute.IsValid())
	{
		// 아이템 효과 (영구적 + 일시적 버프)가 모두 적용된 상태의 스탯값
		float CleanStatValue = SourceASC->GetNumericAttribute(Effect.ScaleAttribute);
		// 모든 아이템 효과 - 일시적 버프 값 = 영구적 (장착한 아이템의 스탯만 남도록)
		CleanStatValue -= AppliedBonus;
		FinalMagnitude += (CleanStatValue * Effect.Magnitude);
	}
	return FinalMagnitude;
}

void UCAP_ItemGameplayAbility::ApplyEffectToTarget(UAbilitySystemComponent* SourceASC,
	UAbilitySystemComponent* TargetASC, const struct FItemEffectPayload& Effect, class UCAP_ItemInstance* ItemInst,
	float FinalMagnitude, int32 TargetStackCount, const FHitResult* HitResult,
	TSubclassOf<UGameplayEffect> MasterGE) const
{
	// 새로운 스펙 바구니 생성
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(ItemInst);
	if (HitResult)
	{
		Context.AddHitResult(*HitResult); // 피격 이펙트 및 데미지 폰트용 보존
	}

	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MasterGE, GetAbilityLevel(), Context);
	if (SpecHandle.IsValid())
	{
		FGameplayTag ValueTag = Effect.TargetStatTag.IsValid() ? Effect.TargetStatTag : FGameplayTag::RequestGameplayTag("Data.Value");
		FGameplayTag StackTag = FGameplayTag::RequestGameplayTag("Data.StackCount");

		// (계산된 최종 값 * 업데이트 된 스택 개수) 한번에 적용
		SpecHandle.Data->SetSetByCallerMagnitude(ValueTag, FinalMagnitude * TargetStackCount);
		// 스택 몇개 쌓았는지 적용
		SpecHandle.Data->SetSetByCallerMagnitude(StackTag, TargetStackCount);
		UE_LOG(LogTemp, Warning,TEXT("현재 스택 = %d"), TargetStackCount);

		// 효과 지속 시간 설정
		if (Effect.Duration > 0.f)
			SpecHandle.Data->SetSetByCallerMagnitude(ItemEffectDurationTag, Effect.Duration);
		// 태그 부여
		if (Effect.DynamicTag.IsValid())
			SpecHandle.Data->DynamicGrantedTags.AddTag(Effect.DynamicTag);

		// 나머지 세팅값 0 초기화 방어
		if (const UGameplayEffect* DefaultGE = MasterGE->GetDefaultObject<UGameplayEffect>())
		{	// 현재 사용중인 GE에서 설정한 모든 Modifier 가져와
			for (const FGameplayModifierInfo& ModInfo : DefaultGE->Modifiers)
			{	// 가져온 Modifier에서 SetbyCaller로 설정한 것만 챙겨와
				if (ModInfo.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
				{	// SetbyCaller의 호출자로 설정한 태그 가져와
					FGameplayTag CallerTag = ModInfo.ModifierMagnitude.GetSetByCallerFloat().DataTag;
					if (CallerTag.IsValid() && CallerTag != ValueTag && CallerTag != ItemEffectDurationTag && CallerTag != StackTag)
					{
						SpecHandle.Data->SetSetByCallerMagnitude(CallerTag, 0.f);
					}
				}
			}
		}
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
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
