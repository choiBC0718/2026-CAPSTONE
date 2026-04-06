// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerController.h"
#include "Widget/HUD/CAP_GameplayWidget.h"
#include "CAP_PlayerCharacter.h"
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
		GameplayWidget->ConfigureAbilities(PlayerCharacter->GetAbilities());
	}
}
