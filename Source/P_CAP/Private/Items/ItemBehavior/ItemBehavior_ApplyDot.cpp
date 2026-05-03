// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemBehavior/ItemBehavior_ApplyDot.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Items/Item/CAP_InventoryComponent.h"
#include "Items/Item/CAP_ItemInstance.h"

void UItemBehavior_ApplyDot::OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	Super::OnEquipped(ItemInst, ASC);
	BindGameplayEvent(ItemInst,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyDot::OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(ItemInst,ASC);
	Super::OnUnequipped(ItemInst, ASC);
}

void UItemBehavior_ApplyDot::OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC,	const struct FGameplayEventData* Payload) const
{
	if (!CheckTriggerCondition(ItemInst, ASC))
		return;
	
	TArray<AActor*> TargetActors = UAbilitySystemBlueprintLibrary::GetAllActorsFromTargetData(Payload->TargetData);
	if (Payload->Target)
		TargetActors.AddUnique(const_cast<AActor*>(Payload->Target.Get()));

	for (AActor* Actor : TargetActors)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);
		if (!TargetASC || TargetASC == ASC)
			continue;
		ApplyDoTToSingleTarget(ItemInst, ASC, TargetASC);
	}
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			InvComp->OnItemEffectTriggered.Broadcast(ItemInst,TriggerEventTag,Cooldown,0.f,0);
	}
}

bool UItemBehavior_ApplyDot::CheckTriggerCondition(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	if (IsOnCooldown(ItemInst,ASC))
		return false;
	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;

	ConsumeCooldown(ItemInst, ASC);
	return true;
}

void UItemBehavior_ApplyDot::ApplyDoTToSingleTarget(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* SourceASC,UAbilitySystemComponent* TargetASC) const
{
	UCAP_AbilitySystemComponent* CAP_ASC = ItemInst->GetCachedASC();
	if (!CAP_ASC || !CAP_ASC->GetGenerics())
		return;
	TSubclassOf<UGameplayEffect> MasterDotEffect = CAP_ASC->GetGenerics()->GetDurationDamageGE(DamageType);
	if (!MasterDotEffect)
		return;

	// 현재 적용중인 GE핸들 확인 먼저
	FActiveGameplayEffectHandle ExistingHandle;
	int32 CurrentItemStack = GetExistingDoTStackCount(ItemInst, TargetASC, MasterDotEffect, ExistingHandle);
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
	Context.AddSourceObject(ItemInst);
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(MasterDotEffect, 1.f, Context);
	if (!SpecHandle.IsValid())
		return;

	if (DynamicTag.IsValid())
	{
		// 같은 GE를 사용하는 Dot 효과에서 구별할 태그
		SpecHandle.Data->DynamicGrantedTags.AddTag(DynamicTag);
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

int32 UItemBehavior_ApplyDot::GetExistingDoTStackCount(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* TargetASC,	TSubclassOf<UGameplayEffect> MasterGE, FActiveGameplayEffectHandle& OutHandle) const
{
	int32 FoundStack = 0;

	FGameplayEffectQuery Query;
	Query.EffectDefinition = MasterGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = TargetASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = TargetASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == ItemInst)
		{
			// 태그가 다르면 건너 뛰어
			if (DynamicTag.IsValid() && !ActiveGE->Spec.DynamicGrantedTags.HasTagExact(DynamicTag))
				continue;
			
			OutHandle = Handle;
			FoundStack = static_cast<int32>(ActiveGE->Spec.GetSetByCallerMagnitude(StackTag,false,0.f));
			break;
		}
	}
	return FoundStack;
}
