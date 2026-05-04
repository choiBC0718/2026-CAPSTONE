// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Item/CAP_WorldItem.h"

#include "CAP_ItemInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

ACAP_WorldItem::ACAP_WorldItem()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshContainer=CreateDefaultSubobject<USceneComponent>(TEXT("MeshContainer"));
	MeshContainer->SetupAttachment(GetRootComponent());
	
	ItemMesh=CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	ItemMesh->SetupAttachment(MeshContainer);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RotatingMovement=CreateDefaultSubobject<URotatingMovementComponent>("RotatingMovement");
	RotatingMovement->SetUpdatedComponent(MeshContainer);
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
	if (ActionType == EInteractAction::Tap)
	{
		if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor))
		{
			if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			{
				InvComp->AddItem(ItemInstance);
				Destroy();
			}
		}
	}else
	{
		UE_LOG(LogTemp,Warning,TEXT("아이템 분해"));
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
	Payload.ActionData.ActionCurrencyType = ECurrencyType::Gold;
	Payload.ActionData.CurrencyAmount = 150;
	return Payload;
}