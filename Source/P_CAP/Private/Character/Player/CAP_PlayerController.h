// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CAP_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY()
	class ACAP_PlayerCharacter* PlayerCharacter;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UCAP_GameplayWidget> GameplayWidgetClass;
	UPROPERTY()
	class UCAP_GameplayWidget* GameplayWidget;
	
	void SpawnGameplayWidget();
};
