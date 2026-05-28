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
	TriggerBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
}

void ADoorActor::InitializeDoor(const FIntPoint& InSourceRoomPos, const FIntPoint& InTargetRoomPos, EDoorDirection InDirection)
{
	SourceRoomPos = InSourceRoomPos;
	TargetRoomPos = InTargetRoomPos;
	Direction = InDirection;
}

void ADoorActor::BeginPlay()
{
	Super::BeginPlay();

	InitializePortalMaterialInstances();
	SetPortalEnabled(true);

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
