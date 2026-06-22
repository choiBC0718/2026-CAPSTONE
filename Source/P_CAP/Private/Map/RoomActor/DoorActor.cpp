// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/DoorActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "Map/Debug/MapManager.h"
#include "Map/NextRoomChoiceManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "P_CAP/P_CAP.h"

ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(Root);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(DoorMesh);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(Root);
	TriggerBox->SetBoxExtent(FVector(80.f, 80.f, 120.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetCollisionResponseToChannel(ECC_PlayerHitbox, ECR_Overlap);

	BackBlocker = CreateDefaultSubobject<UBoxComponent>(TEXT("BackBlocker"));
	BackBlocker->SetupAttachment(Root);
	BackBlocker->SetBoxExtent(BackBlockerExtent);
	BackBlocker->SetRelativeLocation(GetAdjustedBackBlockerRelativeLocation());
	BackBlocker->SetRelativeRotation(GetAdjustedBackBlockerRelativeRotation());
	BackBlocker->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BackBlocker->SetCollisionObjectType(ECC_WorldStatic);
	BackBlocker->SetCollisionResponseToAllChannels(ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_PlayerHitbox, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_EnemyHitbox, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BackBlocker->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}

void ADoorActor::InitializeDoor(const FIntPoint& InSourceRoomPos, const FIntPoint& InTargetRoomPos, EDoorDirection InDirection)
{
	SourceRoomPos = InSourceRoomPos;
	TargetRoomPos = InTargetRoomPos;
	Direction = InDirection;
	ApplyBackBlockerSettings();
}

void ADoorActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyBackBlockerSettings();
}

void ADoorActor::BeginPlay()
{
	Super::BeginPlay();

	InitializePortalMaterialInstances();
	SetPortalEnabled(true);
	ApplyBackBlockerSettings();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::OnTriggerBeginOverlap);
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &ADoorActor::OnTriggerEndOverlap);
	}
}

void ADoorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (FMath::IsNearlyEqual(CurrentPortalAlphaCutoff, TargetPortalAlphaCutoff, KINDA_SMALL_NUMBER))
	{
		SetPortalAlphaCutoff(TargetPortalAlphaCutoff);
		SetActorTickEnabled(false);
		return;
	}

	const float InterpSpeed = 1.f / FMath::Max(PortalTransitionDuration, KINDA_SMALL_NUMBER);
	const float NewAlphaCutoff = FMath::FInterpConstantTo(
		CurrentPortalAlphaCutoff,
		TargetPortalAlphaCutoff,
		DeltaSeconds,
		InterpSpeed);
	SetPortalAlphaCutoff(NewAlphaCutoff);
}

void ADoorActor::SetPortalEnabled(bool bEnabled)
{
	InitializePortalMaterialInstances();

	TargetPortalAlphaCutoff = bEnabled ? 0.f : 1.f;
	SetActorTickEnabled(true);

	if (TriggerBox)
	{
		TriggerBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
}

void ADoorActor::ApplyBackBlockerSettings()
{
	if (!BackBlocker)
	{
		return;
	}

	if (bUseDirectionBasedBackBlockerPlacement)
	{
		ApplyDirectionBasedBackBlockerSettings();
		return;
	}

	if (BackBlocker->GetAttachParent() != PortalMesh)
	{
		BackBlocker->AttachToComponent(PortalMesh, FAttachmentTransformRules::KeepRelativeTransform);
	}

	BackBlocker->SetBoxExtent(BackBlockerExtent);
	BackBlocker->SetRelativeLocation(GetAdjustedBackBlockerRelativeLocation());
	BackBlocker->SetRelativeRotation(GetAdjustedBackBlockerRelativeRotation());
	BackBlocker->SetCollisionEnabled(bEnableBackBlocker ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	BackBlocker->SetCollisionObjectType(ECC_WorldStatic);
	BackBlocker->SetCollisionResponseToAllChannels(ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_PlayerHitbox, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_EnemyHitbox, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BackBlocker->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}

void ADoorActor::ApplyDirectionBasedBackBlockerSettings()
{
	if (!BackBlocker)
	{
		return;
	}

	if (BackBlocker->GetAttachParent() != Root)
	{
		BackBlocker->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform);
	}

	const FVector OutwardDirection = GetDoorOutwardWorldDirection();
	const FVector BlockerWorldLocation = GetActorLocation() + (OutwardDirection * BackBlockerOutwardDistance) + FVector(0.f, 0.f, BackBlockerZOffset);
	const FRotator BlockerWorldRotation(0.f, GetBackBlockerWorldYaw(), 0.f);

	BackBlocker->SetWorldLocation(BlockerWorldLocation);
	BackBlocker->SetWorldRotation(BlockerWorldRotation);
	BackBlocker->SetBoxExtent(BackBlockerExtent);
	BackBlocker->SetCollisionEnabled(bEnableBackBlocker ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	BackBlocker->SetCollisionObjectType(ECC_WorldStatic);
	BackBlocker->SetCollisionResponseToAllChannels(ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_PlayerHitbox, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_EnemyHitbox, ECR_Block);
	BackBlocker->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	BackBlocker->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
}

FVector ADoorActor::GetAdjustedBackBlockerRelativeLocation() const
{
	FVector AdjustedLocation = BackBlockerRelativeLocation;
	if (bFlipBackBlockerForVerticalDoors &&
		(Direction == EDoorDirection::Up || Direction == EDoorDirection::Down))
	{
		AdjustedLocation.X *= -1.f;
	}

	return AdjustedLocation;
}

FRotator ADoorActor::GetAdjustedBackBlockerRelativeRotation() const
{
	if (bRotateBackBlockerForVerticalDoors &&
		(Direction == EDoorDirection::Up || Direction == EDoorDirection::Down))
	{
		return FRotator(0.f, VerticalBackBlockerYawOffset, 0.f);
	}

	return FRotator::ZeroRotator;
}

FVector ADoorActor::GetDoorOutwardWorldDirection() const
{
	switch (Direction)
	{
	case EDoorDirection::Up:
		return FVector(0.f, 1.f, 0.f);

	case EDoorDirection::Down:
		return FVector(0.f, -1.f, 0.f);

	case EDoorDirection::Left:
		return FVector(-1.f, 0.f, 0.f);

	case EDoorDirection::Right:
		return FVector(1.f, 0.f, 0.f);

	default:
		return GetActorForwardVector();
	}
}

float ADoorActor::GetBackBlockerWorldYaw() const
{
	switch (Direction)
	{
	case EDoorDirection::Left:
	case EDoorDirection::Right:
		return 90.f;

	case EDoorDirection::Up:
	case EDoorDirection::Down:
	default:
		return 0.f;
	}
}

void ADoorActor::SetPortalAlphaCutoff(float NewAlphaCutoff)
{
	CurrentPortalAlphaCutoff = FMath::Clamp(NewAlphaCutoff, 0.f, 1.f);

	for (UMaterialInstanceDynamic* MaterialInstance : PortalMaterialInstances)
	{
		if (MaterialInstance)
		{
			MaterialInstance->SetScalarParameterValue(TEXT("Alpha_Cutof"), CurrentPortalAlphaCutoff);
		}
	}
}

void ADoorActor::InitializePortalMaterialInstances()
{
	if (!PortalMesh || PortalMaterialInstances.Num() > 0)
	{
		return;
	}

	const int32 MaterialCount = PortalMesh->GetNumMaterials();
	PortalMaterialInstances.Reserve(MaterialCount);
	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		if (UMaterialInstanceDynamic* MaterialInstance = PortalMesh->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
		{
			PortalMaterialInstances.Add(MaterialInstance);
		}
	}
}

void ADoorActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsProcessingMove)
	{
		return;
	}

	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	bIsProcessingMove = true;

	ANextRoomChoiceManager* ChoiceManager = Cast<ANextRoomChoiceManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ANextRoomChoiceManager::StaticClass()));

	if (ChoiceManager)
	{
		ChoiceManager->RequestEnterRoom(PlayerCharacter, TargetRoomPos, Direction);
	}
	else if (AMapManager* MapManager = Cast<AMapManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapManager::StaticClass())))
	{
		MapManager->RequestMovePlayer(PlayerCharacter, TargetRoomPos, Direction);
	}

	bIsProcessingMove = false;
}

void ADoorActor::OnTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	ANextRoomChoiceManager* ChoiceManager = Cast<ANextRoomChoiceManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ANextRoomChoiceManager::StaticClass()));

	if (ChoiceManager)
	{
		ChoiceManager->CancelCombatRewardChoiceForRoom(TargetRoomPos);
	}
}
