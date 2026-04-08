// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Character/CAP_Character.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_PlayerCharacter : public ACAP_Character
{
	GENERATED_BODY()

public:
	ACAP_PlayerCharacter();
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;


private:
	/**		Components		**/
	UPROPERTY(VisibleAnywhere, Category="View")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category="View")
	class UCameraComponent* Camera;


	/**		Input			**/
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputMappingContext* GameplayIMC;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* MoveIA;

	UPROPERTY(EditDefaultsOnly, Category="Input|Ability")
	TMap<EAbilityInputID, class UInputAction*> AbilityInputActions;


	FVector GetMoveForwardDir();
	FVector GetMoveRightDir();
	void MoveInputHandle(const FInputActionValue& InputActionValue);
	void AbilityInputHandle(const FInputActionValue& InputActionValue, EAbilityInputID AbilityInputID);

	UPROPERTY(VisibleAnywhere, Category="AI|Tracker")
	class UPlayerTrackerComponent* PlayerTracker;
};
