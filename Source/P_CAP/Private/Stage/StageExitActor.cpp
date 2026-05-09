// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/StageExitActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "P_CAP/P_CAP.h"
#include "Stage/StageManager.h"

AStageExitActor::AStageExitActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(Root);
	TriggerBox->SetBoxExtent(FVector(120.f, 120.f, 120.f));
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(Root);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.25f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultPortalMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (DefaultPortalMesh.Succeeded())
	{
		PortalMesh->SetStaticMesh(DefaultPortalMesh.Object);
	}
}

void AStageExitActor::BeginPlay()
{
	Super::BeginPlay();

	if (!StageManager)
	{
		StageManager = Cast<AStageManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AStageManager::StaticClass()));
	}

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AStageExitActor::OnTriggerBeginOverlap);
	}

	SetExitEnabled(bEnabled);
}

void AStageExitActor::SetExitEnabled(bool bNewEnabled)
{
	bEnabled = bNewEnabled;

	if (TriggerBox)
	{
		TriggerBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		TriggerBox->SetGenerateOverlapEvents(bEnabled);
	}

	if (PortalMesh)
	{
		PortalMesh->SetVisibility(bEnabled, true);
		PortalMesh->SetHiddenInGame(!bEnabled, true);
	}
}

void AStageExitActor::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bEnabled || !Cast<APawn>(OtherActor))
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Cyan,
			FString::Printf(TEXT("StageExit overlap: %s"), *GetNameSafe(OtherActor))
		);
	}

	if (!StageManager)
	{
		StageManager = Cast<AStageManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AStageManager::StaticClass()));
	}

	if (!StageManager)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("StageExit: StageManager not found"));
		}
		return;
	}

	StageManager->AdvanceToNextStage();
}
