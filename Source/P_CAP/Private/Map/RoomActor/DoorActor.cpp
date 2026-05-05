// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/DoorActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "Map/Debug/MapManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "P_CAP/P_CAP.h"

ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(Root);
	DoorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADoorActor::OnTriggerBeginOverlap);
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

	AMapManager* MapManager = Cast<AMapManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMapManager::StaticClass()));

	if (MapManager)
	{
		MapManager->RequestMovePlayer(PlayerCharacter, TargetRoomPos, Direction);
	}

	bIsProcessingMove = false;
}

