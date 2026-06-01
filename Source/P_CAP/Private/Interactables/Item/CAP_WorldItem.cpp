// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Item/CAP_WorldItem.h"

#include "CAP_ItemInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Framework/CAP_RewardSettings.h"
#include "GameFramework/RotatingMovementComponent.h"

ACAP_WorldItem::ACAP_WorldItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	ItemMesh=CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	ItemMesh->SetupAttachment(InteractionSphere);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RotatingMovement=CreateDefaultSubobject<URotatingMovementComponent>("RotatingMovement");
	RotatingMovement->SetUpdatedComponent(InteractionSphere);
	RotatingMovement->RotationRate=FRotator(0.f,45.f,0.f);
}

void ACAP_WorldItem::BeginPlay()
{
	Super::BeginPlay();

	if (!ItemInstance && ItemDA)
	{
		ItemInstance = NewObject<UCAP_ItemInstance>(this);
		ItemInstance->Initialize(ItemDA);
	}

	if (ItemDA)
	{
		const UCAP_RewardSettings* RewardSetting = GetDefault<UCAP_RewardSettings>();
		if (RewardSetting->DisassembleRewardDT.IsNull())
			return;

		if (UDataTable* LoadedDT = RewardSetting->DisassembleRewardDT.LoadSynchronous())
		{
			FName Grade = RewardSetting->GetRowNameFromGrade(ItemDA->ItemGrade);
			if (const FDisassembleRewardRow* RewardRow = LoadedDT->FindRow<FDisassembleRewardRow>(Grade,""))
			{
				CachedBaseRewardAmount = RewardRow->ItemRewardAmount;
				CachedRewardCurrencyType = RewardRow->ItemCurrencyType;
			}
		}
	}
}

void ACAP_WorldItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (ItemDA && !ItemDA->ItemMesh.IsNull())
	{
		ItemMesh->SetStaticMesh(ItemDA->ItemMesh.LoadSynchronous());
		ItemMesh->SetRelativeScale3D(ItemDA->MeshScale);
	}
}

void ACAP_WorldItem::Interact(AActor* InsActor, EInteractAction ActionType)
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor);
	UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent();
	if (!Player || !InvComp || !ItemInstance)
		return;
	
	if (ActionType == EInteractAction::Tap)
	{
		if (InvComp->AddItem(ItemInstance))
			Destroy();
	}
	else
	{
		if (InvComp->DisassembleItem(ItemInstance))
			Destroy();
	}
}

FInteractionPayload ACAP_WorldItem::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	Payload.DetailData = ItemInstance;
	Payload.ActionData.ShortActionText = TEXT("줍기");
	Payload.ActionData.LongActionText = TEXT("파괴하기");
	Payload.ActionData.bShowCurrency = true;
	Payload.ActionData.ActionCurrencyType = CachedRewardCurrencyType;
	Payload.ActionData.CurrencyAmount = CachedBaseRewardAmount; 
	
	return Payload;
}

void ACAP_WorldItem::DropItem()
{
	if (BaseCollision)
	{
		FVector DropImpulse = FVector(0.f,0.f, 600.f);
		BaseCollision->AddImpulse(DropImpulse, NAME_None, true);
	}
	SetItemStaticMesh();
}

void ACAP_WorldItem::InitializeItemData(UCAP_ItemDataAsset* NewItemDA)
{
	ItemDA=NewItemDA;
}

void ACAP_WorldItem::SetItemStaticMesh()
{
	if (ItemInstance && ItemInstance->GetItemDA())
	{
		ItemMesh->SetStaticMesh(ItemDA->ItemMesh.LoadSynchronous());
		ItemMesh->SetRelativeScale3D(ItemDA->MeshScale);
	}
}
