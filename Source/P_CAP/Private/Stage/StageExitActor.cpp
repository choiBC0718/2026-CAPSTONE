// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/StageExitActor.h"

#include "Components/BoxComponent.h"
#include "Components/ChildActorComponent.h"
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
	TriggerBox->SetBoxExtent(TriggerExtent);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetGenerateOverlapEvents(true);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(Root);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.25f));

	PortalVisual = CreateDefaultSubobject<UChildActorComponent>(TEXT("PortalVisual"));
	PortalVisual->SetupAttachment(Root);

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
		TriggerBox->SetBoxExtent(TriggerExtent);
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AStageExitActor::OnTriggerBeginOverlap);
	}

	if (PortalVisual)
	{
		PortalVisual->SetChildActorClass(PortalVisualClass);
		PortalVisual->SetVisibility(bEnabled, true);
		PortalVisual->SetHiddenInGame(!bEnabled, true);
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
		const bool bShowDefaultMesh = bEnabled && bUseDefaultPortalMesh;
		PortalMesh->SetVisibility(bShowDefaultMesh, true);
		PortalMesh->SetHiddenInGame(!bShowDefaultMesh, true);
	}

	if (PortalVisual)
	{
		const bool bShowVisual = bEnabled && PortalVisualClass != nullptr;
		PortalVisual->SetVisibility(bShowVisual, true);
		PortalVisual->SetHiddenInGame(!bShowVisual, true);
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
	if (!bEnabled || bIsProcessingExit || !Cast<APawn>(OtherActor))
	{
		return;
	}

	bIsProcessingExit = true;

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
		bIsProcessingExit = false;
		return;
	}

	StageManager->AdvanceToNextStage();
}
