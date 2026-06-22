// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_ApplyDot.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Component/CAP_InventoryComponent.h"

void UItemBehavior_ApplyDot::OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	BindGameplayEvent(StateProvider,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyDot::OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(StateProvider,ASC);
}

void UItemBehavior_ApplyDot::OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC,	const struct FGameplayEventData* Payload) const
{
	if (!CheckTriggerCondition(StateProvider, ASC))
		return;
	
	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(Payload->TargetData);
	if (Payload->Target)
		TargetActors.AddUnique(const_cast<AActor*>(Payload->Target.Get()));

	for (AActor* Actor : TargetActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (!TargetASC || TargetASC == ASC)
			continue;
		ApplyDoTToSingleTarget(StateProvider, ASC, TargetASC);
	}
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			InvComp->OnItemEffectTriggered.Broadcast(StateProvider->GetProviderObject(),TriggerEventTag,Cooldown,0.f,0);
	}
}

bool UItemBehavior_ApplyDot::CheckTriggerCondition(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	if (IsOnCooldown(StateProvider,ASC))
		return false;
	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;

	ConsumeCooldown(StateProvider, ASC);
	return true;
}

void UItemBehavior_ApplyDot::ApplyDoTToSingleTarget(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* SourceASC,UAbilitySystemComponent* TargetASC) const
{
	if (!SourceASC || !SourceASC->GetGenerics())
		return;
	TSubclassOf<UGameplayEffect> MasterDotEffect = SourceASC->GetGenerics()->GetDurationDamageGE(DamageType);
	if (!MasterDotEffect)
		return;

	// 현재 적용중인 GE핸들 확인 먼저
	FActiveGameplayEffectHandle ExistingHandle;
	int32 CurrentItemStack = GetExistingDoTStackCount(StateProvider, TargetASC, MasterDotEffect, ExistingHandle);
	int32 TargetStackCount = FMath::Min(CurrentItemStack+1, MaxStackCount);

	// 기존 핸들 있으면 수치만 덮어씌워
	if (ExistingHandle.IsValid())
	{
		TargetASC->UpdateActiveGameplayEffectSetByCallerMagnitude(ExistingHandle, StackTag,TargetStackCount);
		TargetASC->UpdateActiveGameplayEffectSetByCallerMagnitude(ExistingHandle, BaseDamageTag,BaseTickDamage * TargetStackCount);
		TargetASC->UpdateActiveGameplayEffectSetByCallerMagnitude(ExistingHandle, DamageMultiplierTag,Magnitude);

		if (Duration > 0.f)
		{
			TargetASC->UpdateActiveGameplayEffectSetByCallerMagnitude(ExistingHandle, DurationTag, Duration);
		}
		return;
	}
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(StateProvider->GetProviderObject());
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MasterDotEffect, 1.f, Context);
	if (!SpecHandle.IsValid())
		return;

	if (BehaviorTag.IsValid())
	{
		// 같은 GE를 사용하는 Dot 효과에서 구별할 태그
		SpecHandle.Data->DynamicGrantedTags.AddTag(BehaviorTag);
	}
	
	SpecHandle.Data->SetSetByCallerMagnitude(StackTag, TargetStackCount);
	SpecHandle.Data->SetSetByCallerMagnitude(BaseDamageTag, BaseTickDamage * TargetStackCount);
	SpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierTag, Magnitude);
	if (Duration > 0.f)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DurationTag, Duration);
	}
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

int32 UItemBehavior_ApplyDot::GetExistingDoTStackCount(ICAP_BehaviorStateProvider* StateProvider, UAbilitySystemComponent* TargetASC,	TSubclassOf<UGameplayEffect> MasterGE, FActiveGameplayEffectHandle& OutHandle) const
{
	int32 FoundStack = 0;

	FGameplayEffectQuery Query;
	Query.EffectDefinition = MasterGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == StateProvider->GetProviderObject())
		{
			// 태그가 다르면 건너 뛰어
			if (BehaviorTag.IsValid() && !ActiveGE->Spec.DynamicGrantedTags.HasTagExact(BehaviorTag))
				continue;
			
			OutHandle = Handle;
			FoundStack = static_cast<int32>(ActiveGE->Spec.GetSetByCallerMagnitude(StackTag,false,0.f));
			break;
		}
	}
	return FoundStack;
}
