// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerController.h"
#include "Widget/HUD/CAP_GameplayWidget.h"
#include "CAP_PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

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
		
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Started, this, &ACAP_PlayerController::UIConfirmHandle);
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Ongoing, this, &ACAP_PlayerController::UIConfirmHandle);
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Triggered, this, &ACAP_PlayerController::UIConfirmHandle);
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Completed, this, &ACAP_PlayerController::UIConfirmHandle);
		EnhancedInputComp->BindAction(UIConfirm, ETriggerEvent::Canceled, this, &ACAP_PlayerController::UIConfirmHandle);
	}
}


void ACAP_PlayerController::SpawnGameplayWidget()
{
	GameplayWidget = CreateWidget<UCAP_GameplayWidget>(this, GameplayWidgetClass);
	if (GameplayWidget)
	{
		GameplayWidget->AddToViewport();
	}
}

void ACAP_PlayerController::ToggleCharacterMenu()
{
	if (GameplayWidget)
	{
		// tab 메뉴 열려있지 않으면 열고 게임 일시정지
		if (!GameplayWidget->IsCharacterMenuOpen())
		{
			GameplayWidget->ActivateSwitcher();
		}
		// 열려있다면 탭 전환
		else
		{
			GameplayWidget->SwitchCharacterMenuTab();
		}
	}
}

void ACAP_PlayerController::UINavigationHandle(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal = InputActionValue.Get<FVector2D>();
	if (!GameplayWidget)
		return;

	if (GameplayWidget->IsCharacterMenuOpen())
	{
		GameplayWidget->GetCharacterMenuWidget()->NavigationInput(InputVal);
	}
	else if (GameplayWidget->IsItemSwapMenuOpen())
	{
		GameplayWidget->GetItemSwapWidget()->MoveSelection(InputVal);
	}
}

void ACAP_PlayerController::UIConfirmHandle(const struct FInputActionInstance& Instance)
{
	if (!GameplayWidget)
		return;

	ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
	float ElapsedTime = Instance.GetElapsedTime();

	GameplayWidget->RouteUIConfirmInput(TriggerEvent, ElapsedTime);
}

void ACAP_PlayerController::UICloseHandle(const FInputActionValue& InputActionValue)
{
	if (!GameplayWidget) return;

	if (GameplayWidget->IsItemSwapMenuOpen() || GameplayWidget->IsCharacterMenuOpen())
	{
		GameplayWidget->DeactivateSwitcher();
	}
}