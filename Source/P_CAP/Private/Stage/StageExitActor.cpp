// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/StageExitActor.h"

#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
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
	TriggerBox->SetCollisionResponseToChannel(ECC_PlayerHitbox, ECR_Overlap);

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(Root);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalMesh->SetRelativeScale3D(FVector(1.5f, 1.5f, 0.25f));

	PortalEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PortalEffect"));
	PortalEffect->SetupAttachment(Root);
	PortalEffect->SetAutoActivate(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultPortalMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (DefaultPortalMesh.Succeeded())
	{
		PortalMesh->SetStaticMesh(DefaultPortalMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultPortalEffect(
		TEXT("/Game/PortalSystem/FX/NS_Portal_02.NS_Portal_02"));
	if (DefaultPortalEffect.Succeeded())
	{
		PortalEffect->SetAsset(DefaultPortalEffect.Object);
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
		TriggerBox->OnComponentEndOverlap.AddDynamic(this, &AStageExitActor::OnTriggerEndOverlap);
	}

	SetExitEnabled(bEnabled);
}

void AStageExitActor::SetExitEnabled(bool bNewEnabled)
{
	const bool bWasEnabled = bEnabled;
	bEnabled = bNewEnabled;

	if (TriggerBox)
	{
		if (bEnabled && !bWasEnabled)
		{
			// Ignore overlap events caused by enabling the trigger around a player
			// who is already standing at the exit. They can leave and re-enter to use it.
			bIsTriggerArming = true;
		}
		else if (!bEnabled)
		{
			bIsTriggerArming = false;
			ActorsToIgnoreUntilExit.Empty();
		}

		TriggerBox->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
		TriggerBox->SetGenerateOverlapEvents(bEnabled);

		if (bEnabled && !bWasEnabled)
		{
			TArray<AActor*> OverlappingPawns;
			TriggerBox->GetOverlappingActors(OverlappingPawns, APawn::StaticClass());
			for (AActor* OverlappingPawn : OverlappingPawns)
			{
				ActorsToIgnoreUntilExit.Add(OverlappingPawn);
			}

			GetWorldTimerManager().SetTimerForNextTick(this, &AStageExitActor::FinishTriggerArming);
		}
	}

	if (PortalMesh)
	{
		const bool bShowDefaultMesh = bEnabled && bUseDefaultPortalMesh;
		PortalMesh->SetVisibility(bShowDefaultMesh, true);
		PortalMesh->SetHiddenInGame(!bShowDefaultMesh, true);
	}

	if (PortalEffect)
	{
		const bool bShowEffect = bEnabled && PortalEffect->GetAsset() != nullptr;
		PortalEffect->SetVisibility(bShowEffect, true);
		PortalEffect->SetHiddenInGame(!bShowEffect, true);
		if (bShowEffect)
		{
			PortalEffect->Activate(true);
		}
		else
		{
			PortalEffect->Deactivate();
		}
	}
}

void AStageExitActor::FinishTriggerArming()
{
	bIsTriggerArming = false;
}

void AStageExitActor::OnTriggerEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (OtherActor && !TriggerBox->IsOverlappingActor(OtherActor))
	{
		ActorsToIgnoreUntilExit.Remove(OtherActor);
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

	if (bIsTriggerArming)
	{
		ActorsToIgnoreUntilExit.Add(OtherActor);
		return;
	}

	if (ActorsToIgnoreUntilExit.Contains(OtherActor))
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
