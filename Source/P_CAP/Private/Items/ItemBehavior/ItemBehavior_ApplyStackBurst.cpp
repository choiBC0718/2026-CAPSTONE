// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemBehavior/ItemBehavior_ApplyStackBurst.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Items/Item/CAP_ItemInstance.h"

void UItemBehavior_ApplyStackBurst::OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	Super::OnEquipped(ItemInst, ASC);
	BindGameplayEvent(ItemInst,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyStackBurst::OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(ItemInst,ASC);
	Super::OnUnequipped(ItemInst, ASC);
}

void UItemBehavior_ApplyStackBurst::OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC,const struct FGameplayEventData* Payload) const
{
	if (!Payload || !CheckTriggerCondition(ItemInst, ASC))
		return;

	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(Payload->TargetData);
	if (Payload->Target)
	{
		TargetActors.AddUnique(const_cast<AActor*>(Payload->Target.Get()));
	}

	for (AActor* Actor : TargetActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (!TargetASC || TargetASC == ASC)
			continue;
		ApplyBurstLogicToSingleTarget(ItemInst, ASC, TargetASC);
	}
}

bool UItemBehavior_ApplyStackBurst::CheckTriggerCondition(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	if (!CheckAndConsumeCooldown(ItemInst,ASC))
		return false;

	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;
	
	return true;
}

void UItemBehavior_ApplyStackBurst::ApplyBurstLogicToSingleTarget(UCAP_ItemInstance* ItemInst,UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const
{
	UCAP_AbilitySystemComponent* CAP_ASC = ItemInst->GetCachedASC();
	if (!CAP_ASC || !CAP_ASC->GetGenerics())
		return;
	
	TSubclassOf<UGameplayEffect> MasterMarkGE = CAP_ASC->GetGenerics()->GetItemMarkGE();
	TSubclassOf<UGameplayEffect> MasterInstantDamageGE = CAP_ASC->GetGenerics()->GetInstantDamageGE(DamageType);
	if (!MasterMarkGE || !MasterInstantDamageGE)
		return;

	FActiveGameplayEffectHandle ExistingHandle;
	int32 CurrentItemStack = GetExistingMarkStackCount(ItemInst,TargetASC,MasterMarkGE,ExistingHandle);
	
	if (CurrentItemStack +1 >= BurstStackCount)
	{
		if (ExistingHandle.IsValid())
			TargetASC->RemoveActiveGameplayEffect(ExistingHandle);

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(ItemInst);
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MasterInstantDamageGE, 1.f, Context);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(BaseDamageTag, BaseValue);
			SpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierTag, Magnitude);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
	}
	else
	{
		if (ExistingHandle.IsValid())
			TargetASC->UpdateActiveGameplayEffectSetByCallerMagnitude(ExistingHandle, StackTag, CurrentItemStack+1);
		else
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(ItemInst);
			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MasterMarkGE, 1.f, Context);

			if (SpecHandle.IsValid())
			{
				if (DynamicTag.IsValid())
					SpecHandle.Data->DynamicGrantedTags.AddTag(DynamicTag);

				SpecHandle.Data->SetSetByCallerMagnitude(StackTag, 1.f);
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

int32 UItemBehavior_ApplyStackBurst::GetExistingMarkStackCount(UCAP_ItemInstance* ItemInst,
	UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> MarkGE,FActiveGameplayEffectHandle& OutHandle) const
{
	int32 FoundStack = 0;

	FGameplayEffectQuery Query;
	Query.EffectDefinition = MarkGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == ItemInst)
		{
			if (DynamicTag.IsValid() && !ActiveGE->Spec.DynamicGrantedTags.HasTagExact(DynamicTag))
				continue;
			
			OutHandle = Handle;
			FoundStack = static_cast<int32>(ActiveGE->Spec.GetSetByCallerMagnitude(StackTag, false, 0.f));
			break;
		}
	}
	return FoundStack;
}
