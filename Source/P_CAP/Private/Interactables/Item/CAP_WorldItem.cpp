// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Item/CAP_WorldItem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CAP_ItemInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"

ACAP_WorldItem::ACAP_WorldItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	RootCollision = CreateDefaultSubobject<USphereComponent>("RootCollision");
	SetRootComponent(RootCollision);
	RootScene ->RemoveFromRoot();
	
	MeshContainer=CreateDefaultSubobject<USceneComponent>("MeshContainer");
	MeshContainer->SetupAttachment(InteractionSphere);
	
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
	RootCollision->OnComponentHit.AddDynamic(this, &ACAP_WorldItem::OnRootCollisionHit);
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
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);
		UCAP_AbilitySystemComponent* P_ASC = Cast<UCAP_AbilitySystemComponent>(ASC);
		UCAP_CurrencyComponent* CurrencyComp = Player->GetCurrencyComponent();
		if (!ASC || !P_ASC || !CurrencyComp)
			return;

		const UCAP_AbilitySystemGenerics* Generics = P_ASC->GetGenerics();
		if (Generics && Generics->GetDisassembleRewardDataTable())
		{
			FString EnumString = UEnum::GetValueAsString(ItemDA->ItemGrade);
			UE_LOG(LogTemp,Warning,TEXT("Enum String = %s"), * EnumString);
			FString GradeName = EnumString.RightChop(EnumString.Find(TEXT("::"))+2);
			UE_LOG(LogTemp,Warning,TEXT("Grade Name = %s"), * GradeName);

			FDisassembleRewardRow* Row = Generics->GetDisassembleRewardDataTable()->FindRow<FDisassembleRewardRow>(FName(*GradeName),"");
			if (Row)
			{
				UE_LOG(LogTemp,Warning,TEXT("Found Row"));
				float BonusMul = P_ASC->GetNumericAttribute(UCAP_AttributeSet::GetDisassembleBonusMultiplierAttribute());
				int32 FinalAmount = FMath::RoundToInt(Row->ItemRewardAmount * (1.f + BonusMul));
				CurrencyComp->AddCurrency(Row->ItemCurrencyType, FinalAmount);
			}
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
	Payload.ActionData.ActionCurrencyType = ECurrencyType::Gold;
	Payload.ActionData.CurrencyAmount = 0;
	
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
