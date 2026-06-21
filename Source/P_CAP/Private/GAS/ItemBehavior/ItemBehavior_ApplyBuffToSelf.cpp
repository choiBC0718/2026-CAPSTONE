// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_ApplyBuffToSelf.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"

void UItemBehavior_ApplyBuffToSelf::OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	BindGameplayEvent(StateProvider,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyBuffToSelf::OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(StateProvider,ASC);
}

void UItemBehavior_ApplyBuffToSelf::OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const
{
	if (!TargetStatTag.IsValid())
	{
		UE_LOG(LogTemp,Warning,TEXT("상승시킬 스탯의 태그 설정 필요"));
		return;
	}
	
	if (!CheckTriggerCondition(StateProvider,ASC))
		return;

	// 기존 스택 확인 & 파괴용 핸들
	FActiveGameplayEffectHandle ExistingHandle;
	int32 CurrentItemStack = GetExistingStackCount(StateProvider,ASC,ExistingHandle);
	// 최대치 넘지 않게 설정
	int32 TargetStackCount = FMath::Min(CurrentItemStack+1, MaxStackCount);
	
	// 지속 시간, 스택 갱신하기 위해 기존 GE 파괴
	if (ExistingHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ExistingHandle);
	}
	
	ApplyBuffWithStack(StateProvider,ASC,TargetStackCount);

	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			if (Cooldown > 0.f)
				InvComp->OnItemEffectTriggered.Broadcast(StateProvider->GetProviderObject(),BehaviorTag,Cooldown,Duration,0);
	}
}

bool UItemBehavior_ApplyBuffToSelf::CheckTriggerCondition(ICAP_BehaviorStateProvider* StateProvider,UCAP_AbilitySystemComponent* ASC) const
{
	if (IsOnCooldown(StateProvider,ASC))
		return false;
	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;

	StateProvider->AddBehaviorCount(this,1);
	int32 CurrentCount = StateProvider->GetBehaviorCount(this);
	
	if (CurrentCount < RequiredTriggerCount)
		return false;

	ConsumeCooldown(StateProvider, ASC);
	StateProvider->ResetBehaviorCount(this);
	return true;
}

void UItemBehavior_ApplyBuffToSelf::ApplyBuffWithStack(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, int32 TargetStackCount) const
{
	if (!ASC || !ASC->GetGenerics())	return;
	
	TSubclassOf<UGameplayEffect> DurationBuffGE = ASC->GetGenerics()->GetStatGE(false,bIsMultiplier);
	if (!DurationBuffGE)	return;
	
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(StateProvider->GetProviderObject());

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DurationBuffGE, 1.f, Context);
	if (!SpecHandle.IsValid())
		return;
	
	InitGameplayEffectToDefault(SpecHandle, DurationBuffGE, bIsMultiplier?1.f:0.f);
	SpecHandle.Data->SetSetByCallerMagnitude(StackTag, TargetStackCount);

	float FinalMagnitude;
	float StatScale=1.f;
	if (ScaleAttribute.IsValid())
		StatScale = ASC->GetNumericAttribute(ScaleAttribute);
	
	if (bIsMultiplier)
	{
		FinalMagnitude = 1.f + (Magnitude*StatScale*TargetStackCount);
	}
	else
	{
		FinalMagnitude = Magnitude*StatScale*TargetStackCount;
	}
	
	SpecHandle.Data->SetSetByCallerMagnitude(TargetStatTag, FinalMagnitude);
	SpecHandle.Data->DynamicGrantedTags.AddTag(FGameplayTag::RequestGameplayTag("UI.Buff"));

	if (BehaviorTag.IsValid())
	{
		SpecHandle.Data->DynamicGrantedTags.AddTag(BehaviorTag);
	}
	
	if (Duration>0.f)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DurationTag, Duration);
	}
	
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

int32 UItemBehavior_ApplyBuffToSelf::GetExistingStackCount(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC,FActiveGameplayEffectHandle& OutHandle) const
{
	if (!ASC || !ASC->GetGenerics())	return 0;
	TSubclassOf<UGameplayEffect> DurationBuffGE = ASC->GetGenerics()->GetStatGE(false,bIsMultiplier);
	if (!DurationBuffGE)	return 0;
	
	int32 FoundStack =0;

	FGameplayEffectQuery Query;
	Query.EffectDefinition = DurationBuffGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		// 출처 (SourceObject가 이 효과를 부른 아이템 ItemInst와 동일한지 체크 (다른 인스턴스라면 패스)
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == StateProvider->GetProviderObject())
		{
			if (ActiveGE->Spec.DynamicGrantedTags.HasTagExact(BehaviorTag))
			{
				OutHandle = Handle;
				FoundStack = static_cast<int32>(ActiveGE->Spec.GetSetByCallerMagnitude(StackTag, false, 0.f));
				break;
			}
		}
	}
	return FoundStack;
}
