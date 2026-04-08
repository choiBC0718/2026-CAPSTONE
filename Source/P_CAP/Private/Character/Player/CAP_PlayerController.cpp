// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerController.h"
#include "Widget/HUD/CAP_GameplayWidget.h"
#include "CAP_PlayerCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Widget/Common/CAP_ItemInteraction.h"

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
		EnhancedInputComp->BindAction(InventoryToggleIA, ETriggerEvent::Triggered, this, &ACAP_PlayerController::ToggleCharacterMenu);
		EnhancedInputComp->BindAction(CloseInventoryIA, ETriggerEvent::Triggered, this, &ACAP_PlayerController::CloseCharacterMenu);
	}
}

void ACAP_PlayerController::SetInteractUIVisibility(bool bVisible, const FString& KeyName)
{
	if (GameplayWidget && GameplayWidget->GetInteractionWidget())
	{
		GameplayWidget->GetInteractionWidget()->SetInteractionUIVisibility(bVisible);
		if (bVisible)
		{
			GameplayWidget->GetInteractionWidget()->SetInteractKeyText(KeyName);
		}
	}
}

void ACAP_PlayerController::UpdateInteractProgressUI(float Progress)
{
	if (GameplayWidget && GameplayWidget->GetInteractionWidget())
	{
		GameplayWidget->GetInteractionWidget()->UpdateInteractProgress(Progress);
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
			GameplayWidget->OpenCharacterMenu();
			SetPause(true);
		}
		// 열려있다면 탭 전환
		else
		{
			GameplayWidget->SwitchCharacterMenuTab();
		}
	}
}

void ACAP_PlayerController::CloseCharacterMenu()
{
	if (!GameplayWidget || !GameplayWidget->IsCharacterMenuOpen()) return;
	// tab 메뉴 닫기
	GameplayWidget->CloseCharacterMenu();
	SetPause(false);
}
