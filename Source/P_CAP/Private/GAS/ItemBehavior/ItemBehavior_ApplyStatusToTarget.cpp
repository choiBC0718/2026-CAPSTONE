// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_ApplyStatusToTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"

void UItemBehavior_ApplyStatusToTarget::OnEquipped(ICAP_BehaviorStateProvider* StateProvider,
                                                   UCAP_AbilitySystemComponent* ASC) const
{
	BindGameplayEvent(StateProvider,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyStatusToTarget::OnUnequipped(ICAP_BehaviorStateProvider* StateProvider,
	UCAP_AbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(StateProvider,ASC);
}

void UItemBehavior_ApplyStatusToTarget::OnEventReceived(ICAP_BehaviorStateProvider* StateProvider,UCAP_AbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const
{
	if (StatusTagsToGrant.IsEmpty() || !Payload)
		return;
	if (!CheckTriggerCondition(StateProvider, ASC))
		return;

	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(Payload->TargetData);
	if (TargetActors.Num() > 0 && Payload->Target)
		TargetActors.Add(const_cast<AActor*>(Payload->Target.Get()));

	for (AActor* Actor : TargetActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (!TargetASC || TargetASC == ASC)
			continue;
		
		ApplyStatusToSingleTarget(StateProvider, ASC, TargetASC);
	}
}

bool UItemBehavior_ApplyStatusToTarget::CheckTriggerCondition(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	if (FMath::RandRange(0.f, 100.f) > TriggerChance)
		return false;
	return true;
}

void UItemBehavior_ApplyStatusToTarget::ApplyStatusToSingleTarget(ICAP_BehaviorStateProvider* StateProvider,
	UCAP_AbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const
{
	if (!SourceASC || !SourceASC->GetGenerics())
		return;
	TSubclassOf<UGameplayEffect> StatusGE = SourceASC->GetGenerics()->GetStatGE(false,false);
	if (!StatusGE)
		return;

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(StateProvider->GetProviderObject());

	FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(StatusGE, 1.f, Context);
	if (!Spec.IsValid())
		return;

	InitGameplayEffectToDefault(Spec, StatusGE);
	Spec.Data->DynamicGrantedTags.AppendTags(StatusTagsToGrant);
	if (Duration>0.f)
		Spec.Data->SetSetByCallerMagnitude(DurationTag,Duration);

	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
}
