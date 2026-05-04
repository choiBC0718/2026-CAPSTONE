// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_SpawnProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Interactables/Item/CAP_ItemInstance.h"

void UItemBehavior_SpawnProjectile::OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	Super::OnEquipped(ItemInst, ASC);
	BindGameplayEvent(ItemInst,ASC,TriggerEventTag);
}

void UItemBehavior_SpawnProjectile::OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(ItemInst, ASC);
	Super::OnUnequipped(ItemInst, ASC);
}

void UItemBehavior_SpawnProjectile::OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const
{
	if (!Payload || !SpawnerClass || !ASC)
		return;

	if (!CheckTriggerCondition(ItemInst, ASC))
		return;

	AActor* Instigator = ASC->GetAvatarActor();
	if (!Instigator)
		return;

	FVector SPawnLoc = Instigator->GetActorLocation() + FVector(0.f,0.f,100.f);
	if (Payload->TargetData.Num()>0)
	{
		if (const FHitResult* HitResult = Payload->TargetData.Data[0]->GetHitResult())
		{
			SPawnLoc = HitResult->ImpactPoint;
		}else if (const AActor* TargetActor = Cast<AActor>(Payload->Target))
		{
			SPawnLoc = TargetActor->GetActorLocation();
		}
	}

	FAoESpawnerSetupData FinalSetup = SpawnerData;
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Cast<APawn>(Instigator);
	SpawnParams.Owner = Instigator;

	ACAP_AoESpawner* Spawner = Instigator->GetWorld()->SpawnActor<ACAP_AoESpawner>(SpawnerClass, SPawnLoc, FRotator::ZeroRotator, SpawnParams);
	if (Spawner)
	{
		Spawner->InitializeSpawner(FinalSetup);
		
		FGameplayEventData CastPayload;
		CastPayload.Instigator = Instigator;
		FGameplayTag CastTag = FGameplayTag::RequestGameplayTag("Item.Trigger.Cast.Item");
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Instigator,CastTag,CastPayload);
	}
}

bool UItemBehavior_SpawnProjectile::CheckTriggerCondition(UCAP_ItemInstance* ItemInst,UAbilitySystemComponent* ASC) const
{
	if (IsOnCooldown(ItemInst, ASC))
		return false;
	if (FMath::RandRange(0.f, 100.f) > TriggerChance)
		return false;
	
	int32& CurrentCount = ItemInst->BehaviorCounters.FindOrAdd(this);
	CurrentCount++;

	bool bWillFire = (CurrentCount >= RequiredTriggerCount);
	if (!bWillFire)
	{
		if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
		{
			if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
				InvComp->OnItemEffectTriggered.Broadcast(ItemInst,TriggerEventTag,Cooldown,-1.f,CurrentCount);
		}
		return false;
	}

	ConsumeCooldown(ItemInst, ASC);
	CurrentCount = 0;
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
		{
			InvComp->OnItemEffectTriggered.Broadcast(ItemInst, TriggerEventTag, Cooldown, 0.f, CurrentCount);
		}
	}
	return true;
}
