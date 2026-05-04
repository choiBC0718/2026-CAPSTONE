// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_ApplyDebuffToTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Interactables/Item/CAP_ItemInstance.h"

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
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			InvComp->OnItemEffectTriggered.Broadcast(ItemInst,TriggerEventTag,Cooldown,0.f,0);
	}
}

bool UItemBehavior_ApplyDebuffToTarget::CheckTriggerCondition(UCAP_ItemInstance* ItemInst,UAbilitySystemComponent* ASC) const
{
	if (IsOnCooldown(ItemInst,ASC))
		return false;
	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;
	
	int32& CurrentCount = ItemInst->BehaviorCounters.FindOrAdd(this);
	CurrentCount++;
	if (CurrentCount < RequiredTriggerCount)
		return false;

	ConsumeCooldown(ItemInst,ASC);
	CurrentCount=0;
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
	if (!ScaleAttribute.IsValid())
	{
		UE_LOG(LogTemp,Warning,TEXT("어떤 스탯에 영향을 받아 감소시킬지 설정해(ScaleAttribute)"));
		return;
	}

	// 타겟에게 적용되던 GE가 있으면 삭제하고 새로운 GE 부여
	FActiveGameplayEffectHandle ExistingHandle = GetExistingDebuffHandle(ItemInst,TargetASC,DurationDebuffGE);
	if (ExistingHandle.IsValid())
	{
		TargetASC->RemoveActiveGameplayEffect(ExistingHandle);
	}
	
	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddSourceObject(ItemInst);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DurationDebuffGE, 1.f, Context);
	if (!SpecHandle.IsValid())
		return;
	
	InitGameplayEffectToZero(SpecHandle, DurationDebuffGE);
	
	float CleanStatValue = TargetASC->GetNumericAttribute(ScaleAttribute);
	float FinalMagnitude = Magnitude * CleanStatValue;
	
	SpecHandle.Data->SetSetByCallerMagnitude(TargetStatTag, -FinalMagnitude);

	if (Duration > 0.f)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DurationTag, Duration);
	}
	SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}

FActiveGameplayEffectHandle UItemBehavior_ApplyDebuffToTarget::GetExistingDebuffHandle(UCAP_ItemInstance* ItemInst,
	UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> MasterGE) const
{
	FActiveGameplayEffectHandle FoundHandle;
	FGameplayEffectQuery Query;
	Query.EffectDefinition = MasterGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == ItemInst)
		{
			FoundHandle = Handle;
			break;
		}
	}
	return FoundHandle;
}
