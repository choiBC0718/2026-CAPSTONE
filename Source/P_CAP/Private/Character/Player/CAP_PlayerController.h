// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "Widget/HUD/CAP_GameplayWidget.h"
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
	virtual void SetupInputComponent() override;

	FORCEINLINE UCAP_GameplayWidget* GetGameplayWidget() const { return GameplayWidget; }

private:
	UPROPERTY()
	class ACAP_PlayerCharacter* PlayerCharacter;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UCAP_GameplayWidget> GameplayWidgetClass;
	UPROPERTY()
	class UCAP_GameplayWidget* GameplayWidget;
	
	void SpawnGameplayWidget();
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputMappingContext* UIInputMapping;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* InventoryToggleIA;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* UICloseIA;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* UINavigation;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* UIConfirm;

	UFUNCTION()
	void ToggleCharacterMenu();
	UFUNCTION()
	void UINavigationHandle(const FInputActionValue& InputActionValue);
	UFUNCTION()
	void UIConfirmHandle(const struct FInputActionInstance& Instance);
	UFUNCTION()
	void UICloseHandle(const FInputActionValue& InputActionValue);
};
