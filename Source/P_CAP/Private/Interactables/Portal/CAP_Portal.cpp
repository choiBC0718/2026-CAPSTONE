// Fill out your copyright notice in the Description page of Project Settings.

#include "Interactables/Portal/CAP_Portal.h"

#include "Blueprint/UserWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Stage/StageLoadingWidget.h"
#include "TimerManager.h"

ACAP_Portal::ACAP_Portal()
{
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>("PortalMesh");
	PortalMesh->SetupAttachment(InteractionSphere);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalMesh->SetRelativeLocation(FVector(0.f, 0.f, -130.f));
	PortalMesh->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
}

void ACAP_Portal::BeginPlay()
{
	Super::BeginPlay();

	if (PortalMesh)
	{
		PortalMesh->SetVisibility(bShowPortalMesh, true);
		PortalMesh->SetHiddenInGame(!bShowPortalMesh, true);
	}
}

void ACAP_Portal::Interact(AActor* InsActor, EInteractAction ActionType)
{
	Super::Interact(InsActor, ActionType);

	if (bIsOpeningLevel)
	{
		return;
	}

	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor))
	{
		Player->SaveProgressionBeforeChangeLevel();
	}

	bIsOpeningLevel = true;

	if (LoadingWidgetClass)
	{
		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			ActiveLoadingWidget = CreateWidget<UStageLoadingWidget>(PlayerController, LoadingWidgetClass);
			if (ActiveLoadingWidget)
			{
				ActiveLoadingWidget->SetLoadingProgress(0.f);
				ActiveLoadingWidget->AddToViewport(1000);
				PlayerController->SetIgnoreMoveInput(true);
				PlayerController->SetIgnoreLookInput(true);
			}
		}
	}

	if (UWorld* World = GetWorld())
	{
		if (OpenLevelDelay > 0.f && ActiveLoadingWidget)
		{
			LoadingElapsedTime = 0.f;
			World->GetTimerManager().SetTimer(
				LoadingProgressTimerHandle,
				this,
				&ACAP_Portal::UpdateLoadingProgress,
				0.02f,
				true);
			return;
		}
	}

	OpenTargetLevel();
}

FInteractionPayload ACAP_Portal::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	Payload.ActionData.ShortActionText = InteractionText.ToString();
	return Payload;
}

void ACAP_Portal::UpdateLoadingProgress()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		OpenTargetLevel();
		return;
	}

	const float SafeDuration = FMath::Max(0.01f, OpenLevelDelay);
	LoadingElapsedTime += 0.02f;

	const float Progress = FMath::Clamp(LoadingElapsedTime / SafeDuration, 0.f, 1.f);
	if (ActiveLoadingWidget)
	{
		ActiveLoadingWidget->SetLoadingProgress(Progress);
	}

	if (Progress >= 1.f)
	{
		World->GetTimerManager().ClearTimer(LoadingProgressTimerHandle);
		OpenTargetLevel();
	}
}

void ACAP_Portal::OpenTargetLevel()
{
	if (NextStageName.IsNone())
	{
		bIsOpeningLevel = false;
		return;
	}

	UGameplayStatics::OpenLevel(this, NextStageName);
}
