// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemBehavior/ItemBehavior_ApplyDebuffToTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Items/Item/CAP_ItemInstance.h"

void UItemBehavior_ApplyDebuffToTarget::OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	Super::OnEquipped(ItemInst, ASC);
	BindGameplayEvent(ItemInst,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyDebuffToTarget::OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(ItemInst,ASC);
	Super::OnUnequipped(ItemInst, ASC);
}

void UItemBehavior_ApplyDebuffToTarget::OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC,const struct FGameplayEventData* Payload) const
{
	if (!TargetStatTag.IsValid() || !Payload)
		return;

	if (!CheckTriggerCondition(ItemInst,ASC))
		return;

	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(Payload->TargetData);
	if (TargetActors.Num() > 0 && Payload->Target)
		TargetActors.Add(const_cast<AActor*>(Payload->Target.Get()));

	for (AActor* Actor : TargetActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (!TargetASC || TargetASC == ASC)
			continue;
		ApplyDebuffToSingleTarget(ItemInst,ASC,TargetASC);
	}
}

bool UItemBehavior_ApplyDebuffToTarget::CheckTriggerCondition(UCAP_ItemInstance* ItemInst,UAbilitySystemComponent* ASC) const
{
	int32& CurrentCount = ItemInst->BehaviorCounters.FindOrAdd(this);
	CurrentCount++;
	if (CurrentCount < RequiredTriggerCount)
		return false;

	if (!CheckAndConsumeCooldown(ItemInst,ASC))
		return false;
	
	CurrentCount=0;
	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;
	
	return true;
}

void UItemBehavior_ApplyDebuffToTarget::ApplyDebuffToSingleTarget(UCAP_ItemInstance* ItemInst,UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const
{
	UCAP_AbilitySystemComponent* CAP_ASC = ItemInst->GetCachedASC();
	if (!CAP_ASC || !CAP_ASC->GetGenerics())
		return;
	TSubclassOf<UGameplayEffect> DurationDebuffGE = CAP_ASC->GetGenerics()->GetItemStatDurationEffect();
	if (!DurationDebuffGE)
		return;

	FActiveGameplayEffectHandle ExistingHandle;
	int32 CurrentItemStack = GetExistingStackCountOnTarget(ItemInst, TargetASC, DurationDebuffGE, ExistingHandle);

	int32 TargetStackCount = FMath::Min(CurrentItemStack + 1, MaxStackCount);

	if (ExistingHandle.IsValid())
		TargetASC->RemoveActiveGameplayEffect(ExistingHandle);
	
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(ItemInst);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DurationDebuffGE, 1.f, Context);
	if (!SpecHandle.IsValid())
		return;
	
	InitGameplayEffectToZero(SpecHandle, DurationDebuffGE);
	
	FGameplayTag StackTag = FGameplayTag::RequestGameplayTag("Data.StackCount");
	SpecHandle.Data->SetSetByCallerMagnitude(StackTag, TargetStackCount);

	float FinalMagnitude = BaseValue;
	if (ScaleAttribute.IsValid())
	{
		float CleanStatValue = TargetASC->GetNumericAttribute(ScaleAttribute);
		FinalMagnitude += (CleanStatValue * Magnitude);
	}
	SpecHandle.Data->SetSetByCallerMagnitude(TargetStatTag, -FinalMagnitude * TargetStackCount);

	if (Duration > 0.f)
	{
		FGameplayTag DurationTag = FGameplayTag::RequestGameplayTag("Data.ItemEffect.Duration");
		SpecHandle.Data->SetSetByCallerMagnitude(DurationTag, Duration);
	}
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

int32 UItemBehavior_ApplyDebuffToTarget::GetExistingStackCountOnTarget(UCAP_ItemInstance* ItemInst,
	UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> MasterGE,FActiveGameplayEffectHandle& OutHandle) const
{
	int32 FoundStack =0;
	FGameplayTag StackTag = FGameplayTag::RequestGameplayTag("Data.StackCount");

	FGameplayEffectQuery Query;
	Query.EffectDefinition = MasterGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == ItemInst)
		{
			OutHandle = Handle;
			FoundStack = static_cast<int32>(ActiveGE->Spec.GetSetByCallerMagnitude(StackTag,false,0.f));
			break;
		}
	}
	return FoundStack;
}
