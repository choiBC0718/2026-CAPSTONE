// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerController.h"
#include "Widget/HUD/CAP_GameplayWidget.h"
#include "Widget/HUD/CAP_MinimapWidget.h"
#include "CAP_PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Map/Debug/MapManager.h"

void ACAP_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PlayerCharacter = Cast<ACAP_PlayerCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SpawnGameplayWidget();
	}
}

void ACAP_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (InputSubsystem)
	{
		InputSubsystem->RemoveMappingContext(UIInputMapping);
		InputSubsystem->AddMappingContext(UIInputMapping, 1);
	}

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(InventoryToggleIA, ETriggerEvent::Started, this, &ACAP_PlayerController::ToggleCharacterMenu);
		EnhancedInputComp->BindAction(UICloseIA, ETriggerEvent::Started, this, &ACAP_PlayerController::UICloseHandle);
		EnhancedInputComp->BindAction(UINavigation, ETriggerEvent::Started, this, &ACAP_PlayerController::UINavigationHandle);
		
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Started, this, &ACAP_PlayerController::HandleUIConfirmStarted);
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Ongoing, this, &ACAP_PlayerController::HandleUIConfirmOngoing);
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Triggered, this, &ACAP_PlayerController::HandleUIConfirmTriggered);
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Completed, this, &ACAP_PlayerController::HandleUIConfirmCompleted);
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Canceled, this, &ACAP_PlayerController::HandleUIConfirmCanceled);
	}

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Z, IE_Pressed, this, &ACAP_PlayerController::ExpandMinimap);
		InputComponent->BindKey(EKeys::Z, IE_Released, this, &ACAP_PlayerController::RestoreMinimapSize);
	}
}

void ACAP_PlayerController::TeleportToBossRoom()
{
	AMapManager* MapManager = Cast<AMapManager>(
		UGameplayStatics::GetActorOfClass(this, AMapManager::StaticClass()));
	if (!MapManager)
	{
		ClientMessage(TEXT("TeleportToBossRoom: MapManager not found."));
		return;
	}

	ARoomActor* BossRoom = MapManager->FindSpawnedRoomByGridPos(
		MapManager->GetCurrentLayout().BossRoomPos);
	if (!BossRoom)
	{
		ClientMessage(TEXT("TeleportToBossRoom: boss room has not been generated."));
		return;
	}

	APawn* PlayerPawn = GetPawn();
	if (!PlayerPawn)
	{
		ClientMessage(TEXT("TeleportToBossRoom: possessed pawn not found."));
		return;
	}

	const FVector Destination = BossRoom->GetActorLocation() + FVector(0.f, 0.f, 150.f);
	PlayerPawn->SetActorLocation(Destination, false, nullptr, ETeleportType::TeleportPhysics);
	ClientMessage(TEXT("Teleported to the boss room."));
}


void ACAP_PlayerController::SpawnGameplayWidget()
{
	GameplayWidget = CreateWidget<UCAP_GameplayWidget>(this, GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
	}
	MinimapWidget = CreateWidget<UCAP_MinimapWidget>(this, UCAP_MinimapWidget::StaticClass());
	if (MinimapWidget)
	{
		MinimapWidget->AddToViewport(5);
	}
}

void ACAP_PlayerController::ToggleCharacterMenu()
{
	if (GameplayWidget)
	{
		GameplayWidget->ToggleCharacterMenu();

	}
}

void ACAP_PlayerController::ExpandMinimap()
{
	if (MinimapWidget)
	{
		MinimapWidget->SetExpandedMap(true);
	}
}

void ACAP_PlayerController::RestoreMinimapSize()
{
	if (MinimapWidget)
	{
		MinimapWidget->SetExpandedMap(false);
	}
}

void ACAP_PlayerController::UINavigationHandle(const FInputActionValue& InputActionValue)
{
	if (GameplayWidget)
	{
		GameplayWidget->UINavigationHandle(InputActionValue.Get<FVector2D>());
	}
}

void ACAP_PlayerController::UICloseHandle(const FInputActionValue& InputActionValue)
{
	if (GameplayWidget)
	{
		GameplayWidget->HideMenu();
	}
}

void ACAP_PlayerController::HandleUIConfirmStarted(const FInputActionInstance& InputInst)
{
	if (GameplayWidget)
	{
		//UE_LOG(LogTemp,Warning,TEXT("Started"));
		GameplayWidget->RouteUIConfirmInput(ETriggerEvent::Started, InputInst.GetElapsedTime());
	}
}

void ACAP_PlayerController::HandleUIConfirmOngoing(const FInputActionInstance& InputInst)
{
	if (GameplayWidget)
	{
		//UE_LOG(LogTemp,Warning,TEXT("Ongoing"));
		GameplayWidget->RouteUIConfirmInput(ETriggerEvent::Ongoing, InputInst.GetElapsedTime());
	}
}

void ACAP_PlayerController::HandleUIConfirmTriggered(const FInputActionInstance& InputInst)
{
	if (GameplayWidget)
	{
		//UE_LOG(LogTemp,Warning,TEXT("Triggered"));
		GameplayWidget->RouteUIConfirmInput(ETriggerEvent::Triggered, InputInst.GetElapsedTime());
	}
}

void ACAP_PlayerController::HandleUIConfirmCompleted(const FInputActionInstance& InputInst)
{
	if (GameplayWidget)
	{
		//UE_LOG(LogTemp,Warning,TEXT("Completed"));
		GameplayWidget->RouteUIConfirmInput(ETriggerEvent::Completed, InputInst.GetElapsedTime());
	}
}

void ACAP_PlayerController::HandleUIConfirmCanceled(const FInputActionInstance& InputInst)
{
	if (GameplayWidget)
	{
		//UE_LOG(LogTemp,Warning,TEXT("Canceled"));
		GameplayWidget->RouteUIConfirmInput(ETriggerEvent::Canceled, InputInst.GetElapsedTime());
	}
}
