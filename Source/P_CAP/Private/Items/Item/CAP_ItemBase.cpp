// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item/CAP_ItemBase.h"

#include "CAP_InventoryComponent.h"
#include "CAP_ItemInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"

ACAP_ItemBase::ACAP_ItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh=CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	ItemMesh->SetupAttachment(MeshContainer);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACAP_ItemBase::BeginPlay()
{
	Super::BeginPlay();

	if (!ItemInstance && ItemDA)
	{
		ItemInstance = NewObject<UCAP_ItemInstance>(this);
		ItemInstance->Initialize(ItemDA);
	}
}

void ACAP_ItemBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ItemDA && !ItemDA->ItemMesh.IsNull())
	{
		ItemMesh->SetStaticMesh(ItemDA->ItemMesh.LoadSynchronous());
		ItemMesh->SetRelativeScale3D(ItemDA->MeshScale);
	}
}

void ACAP_ItemBase::InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter)
{
	Super::InteractEquip(PlayerCharacter);
	
	if (PlayerCharacter && ItemInstance)
	{
		UCAP_InventoryComponent* InvComp = PlayerCharacter->GetComponentByClass<UCAP_InventoryComponent>();
		if (InvComp)
		{
			ItemInstance->Rename(nullptr,InvComp);
			if (InvComp->AddItem(ItemInstance))
			{
				Destroy();
			}
		}
	}
}

void ACAP_ItemBase::InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter)
{
	Super::InteractDisassemble(PlayerCharacter);
	Destroy();
}
