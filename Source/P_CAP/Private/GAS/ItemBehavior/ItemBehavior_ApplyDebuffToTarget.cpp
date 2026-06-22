// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_ApplyDebuffToTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"

void UItemBehavior_ApplyDebuffToTarget::OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	BindGameplayEvent(StateProvider,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyDebuffToTarget::OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(StateProvider,ASC);
}

void UItemBehavior_ApplyDebuffToTarget::OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC,const struct FGameplayEventData* Payload) const
{
	if (!TargetStatTag.IsValid() || !Payload)
		return;

	if (!CheckTriggerCondition(StateProvider,ASC))
		return;

	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(Payload->TargetData);
	if (TargetActors.Num() > 0 && Payload->Target)
		TargetActors.Add(const_cast<AActor*>(Payload->Target.Get()));

	for (AActor* Actor : TargetActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (!TargetASC || TargetASC == ASC)
			continue;
		ApplyDebuffToSingleTarget(StateProvider,ASC,TargetASC);
	}
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			InvComp->OnItemEffectTriggered.Broadcast(StateProvider->GetProviderObject(),BehaviorTag,Cooldown,0.f,0);
	}
}

bool UItemBehavior_ApplyDebuffToTarget::CheckTriggerCondition(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	if (IsOnCooldown(StateProvider,ASC))
		return false;
	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;

	StateProvider->AddBehaviorCount(this,1);
	int32 CurrentCount = StateProvider->GetBehaviorCount(this);
	
	if (CurrentCount < RequiredTriggerCount)
		return false;

	ConsumeCooldown(StateProvider,ASC);
	StateProvider->ResetBehaviorCount(this);
	return true;
}

void UItemBehavior_ApplyDebuffToTarget::ApplyDebuffToSingleTarget(ICAP_BehaviorStateProvider* StateProvider,UCAP_AbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC) const
{
	if (!SourceASC || !SourceASC->GetGenerics())
		return;
	
	TSubclassOf<UGameplayEffect> DurationDebuffGE = SourceASC->GetGenerics()->GetStatGE(false,false);
	if (!DurationDebuffGE)
		return;
	if (!ScaleAttribute.IsValid())
	{
		UE_LOG(LogTemp,Warning,TEXT("어떤 스탯에 영향을 받아 감소시킬지 설정해(ScaleAttribute)"));
		return;
	}

	// 타겟에게 적용되던 GE가 있으면 삭제하고 새로운 GE 부여
	FActiveGameplayEffectHandle ExistingHandle = GetExistingDebuffHandle(StateProvider,TargetASC,DurationDebuffGE);
	if (ExistingHandle.IsValid())
	{
		TargetASC->RemoveActiveGameplayEffect(ExistingHandle);
	}
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(StateProvider->GetProviderObject());
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DurationDebuffGE, 1.f, Context);
	if (!SpecHandle.IsValid())
		return;
	
	InitGameplayEffectToDefault(SpecHandle, DurationDebuffGE);
	
	float CleanStatValue = TargetASC->GetNumericAttribute(ScaleAttribute);
	float FinalMagnitude = Magnitude * CleanStatValue;
	
	SpecHandle.Data->SetSetByCallerMagnitude(TargetStatTag, -FinalMagnitude);

	if (Duration > 0.f)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DurationTag, Duration);
	}
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

FActiveGameplayEffectHandle UItemBehavior_ApplyDebuffToTarget::GetExistingDebuffHandle(ICAP_BehaviorStateProvider* StateProvider,
	UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> MasterGE) const
{
	FActiveGameplayEffectHandle FoundHandle;
	FGameplayEffectQuery Query;
	Query.EffectDefinition = MasterGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == StateProvider->GetProviderObject())
		{
			FoundHandle = Handle;
			break;
		}
	}
	return FoundHandle;
}
