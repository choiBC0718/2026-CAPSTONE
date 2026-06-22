// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_ApplyStackBurst.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Interface/CAP_TargetUIInterface.h"

void UItemBehavior_ApplyStackBurst::OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	BindGameplayEvent(StateProvider,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyStackBurst::OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(StateProvider,ASC);
}

void UItemBehavior_ApplyStackBurst::OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC,const struct FGameplayEventData* Payload) const
{
	if (!Payload || !CheckTriggerCondition(StateProvider, ASC))
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
		ApplyBurstLogicToSingleTarget(StateProvider, ASC, TargetASC);
	}
}

bool UItemBehavior_ApplyStackBurst::CheckTriggerCondition(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;
	
	return true;
}

void UItemBehavior_ApplyStackBurst::ApplyBurstLogicToSingleTarget(ICAP_BehaviorStateProvider* StateProvider,UCAP_AbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const
{
	if (!SourceASC || !SourceASC->GetGenerics())
		return;
	
	AActor* TargetActor = TargetASC->GetAvatarActor();
	ICAP_TargetUIInterface* TargetUI = Cast<ICAP_TargetUIInterface>(TargetActor);
	if (!TargetActor || !TargetUI)
		return;

	float CurrentTime = TargetActor->GetWorld()->GetTimeSeconds();
	float TargetLastTime = StateProvider->GetBehaviorTargetCooldown(this, TargetActor);
	if (TargetLastTime > CurrentTime)
		return;
	
	TSubclassOf<UGameplayEffect> MasterMarkGE = SourceASC->GetGenerics()->GetItemMarkGE();
	TSubclassOf<UGameplayEffect> MasterInstantDamageGE = SourceASC->GetGenerics()->GetInstantDamageGE(DamageType);
	if (!MasterMarkGE || !MasterInstantDamageGE)
		return;

	FActiveGameplayEffectHandle ExistingHandle;
	int32 CurrentItemStack = GetExistingMarkStackCount(StateProvider,TargetASC,MasterMarkGE,ExistingHandle);
	
	if (CurrentItemStack +1 >= BurstStackCount)
	{	//Burst 로직
		if (ExistingHandle.IsValid())
			TargetASC->RemoveActiveGameplayEffect(ExistingHandle);

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(StateProvider->GetProviderObject());
		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MasterInstantDamageGE, 1.f, Context);

		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(BaseDamageTag, BaseValue);
			SpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierTag, Magnitude);
			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
		}
		StateProvider->SetBehaviorTargetCooldown(this, TargetActor, CurrentTime+Cooldown);
		TargetUI->UpdateStackUI(BehaviorTag, 0, BurstStackCount);
	}
	else
	{
		// 스택 쌓기 로직
		if (ExistingHandle.IsValid())
			TargetASC->UpdateActiveGameplayEffectSetByCallerMagnitude(ExistingHandle, StackTag, CurrentItemStack+1);
		else
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(StateProvider->GetProviderObject());
			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MasterMarkGE, 1.f, Context);

			if (SpecHandle.IsValid())
			{
				if (BehaviorTag.IsValid())
					SpecHandle.Data->DynamicGrantedTags.AddTag(BehaviorTag);

				SpecHandle.Data->SetSetByCallerMagnitude(StackTag, CurrentItemStack+1);
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
		TargetUI->UpdateStackUI(BehaviorTag, CurrentItemStack+1, BurstStackCount);
	}
}

int32 UItemBehavior_ApplyStackBurst::GetExistingMarkStackCount(ICAP_BehaviorStateProvider* StateProvider,
	UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> MarkGE,FActiveGameplayEffectHandle& OutHandle) const
{
	int32 FoundStack = 0;

	FGameplayEffectQuery Query;
	Query.EffectDefinition = MarkGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == StateProvider->GetProviderObject())
		{
			if (BehaviorTag.IsValid() && !ActiveGE->Spec.DynamicGrantedTags.HasTagExact(BehaviorTag))
				continue;
			
			OutHandle = Handle;
			FoundStack = static_cast<int32>(ActiveGE->Spec.GetSetByCallerMagnitude(StackTag, false, 0.f));
			break;
		}
	}
	return FoundStack;
}
