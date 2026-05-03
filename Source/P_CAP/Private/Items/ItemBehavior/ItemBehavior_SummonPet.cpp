// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ItemBehavior/ItemBehavior_SummonPet.h"

#include "AbilitySystemComponent.h"
#include "Character/Pet/CAP_FlyingPetPawn.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Items/Item/CAP_InventoryComponent.h"
#include "Kismet/GameplayStatics.h"

void UItemBehavior_SummonPet::OnEquipped(class UCAP_ItemInstance* ItemInst, class UAbilitySystemComponent* ASC) const
{
	Super::OnEquipped(ItemInst, ASC);

	AActor* OwnerActor = ASC->GetAvatarActor();
	UWorld* World = OwnerActor->GetWorld();
	if (!OwnerActor || !PetPawnClass || !World)
		return;
	
	TArray<AActor*> ExistingPets;
	UGameplayStatics::GetAllActorsOfClass(World, ACAP_FlyingPetPawn::StaticClass(), ExistingPets);

	int32 OwnedPetCount = 0;
	for (AActor* PetActor : ExistingPets)
	{
		ACAP_FlyingPetPawn* Pet = Cast<ACAP_FlyingPetPawn>(PetActor);
		if (Pet && Pet->Player == OwnerActor)
			OwnedPetCount++;
	}
	
	FTransform SpawnTrans = OwnerActor->GetTransform();
	ACAP_FlyingPetPawn* NewPet = World->SpawnActorDeferred<ACAP_FlyingPetPawn>(PetPawnClass, SpawnTrans);
	if (NewPet)
	{
		NewPet->InitializePet(OwnerActor);
		int32 SlotIdx = FMath::Clamp(OwnedPetCount, 0, PetOffsetSlots.Num() - 1);
		NewPet->SetVisualAndOffset(PetSkeletalMesh, PetOffsetSlots[SlotIdx]);
		NewPet->SetStats(DamageType, BaseDamage, DamageMultiplier);
		NewPet->FinishSpawning(SpawnTrans);
		SpawnedPet = NewPet;
	}
	if (ACAP_PlayerCharacter* PlayerChar = Cast<ACAP_PlayerCharacter>(OwnerActor))
	{
		if (UCAP_InventoryComponent* InvComp = PlayerChar->GetInventoryComponent())
		{
			InvComp->OnItemEffectTriggered.Broadcast(ItemInst, FGameplayTag::EmptyTag, 0.f, -1.f, 0);
		}
	}
}

void UItemBehavior_SummonPet::OnUnequipped(class UCAP_ItemInstance* ItemInst, class UAbilitySystemComponent* ASC) const
{
	if (SpawnedPet.IsValid())
	{
		SpawnedPet->Destroy();
		SpawnedPet.Reset();
	}
	Super::OnUnequipped(ItemInst, ASC);
}
