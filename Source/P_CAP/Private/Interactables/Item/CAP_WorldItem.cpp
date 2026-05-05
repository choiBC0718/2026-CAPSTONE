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
	
	RootCollision = CreateDefaultSubobject<USphereComponent>("RootCollision");
	SetRootComponent(RootCollision);
	RootCollision->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	RootCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	RootCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	RootCollision->SetSimulatePhysics(true);

	RootScene->SetupAttachment(RootCollision);
	
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
	RootCollision->OnComponentHit.AddDynamic(this, &ACAP_WorldItem::OnRootCollisionHit);

	if (ItemDA)
	{
		const UCAP_RewardSettings* RewardSetting = GetDefault<UCAP_RewardSettings>();
		if (const FDisassembleRewardRow* Row = RewardSetting->DisassembleRewardMap.Find(ItemDA->ItemGrade))
		{
			CachedBaseRewardAmount = Row->ItemRewardAmount;
			CachedRewardCurrencyType = Row->ItemCurrencyType;
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
	if (!Player)
		return;
	
	if (ActionType == EInteractAction::Tap)
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
		{
			InvComp->AddItem(ItemInstance);
			Destroy();
		}
	}
	else
	{
		if (ItemDA)
		{
			if (UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent())
				CurrComp->ProcessDisassembleReward(ItemDA->ItemGrade, ECurrencyType::Gold);
		}
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
	if (RootCollision)
	{
		FVector DropImpulse = FVector(0.f,0.f, 600.f);
		RootCollision->AddImpulse(DropImpulse, NAME_None, true);
	}
}

void ACAP_WorldItem::OnRootCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	RootCollision->SetSimulatePhysics(false);
}
