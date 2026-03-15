// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CAP_Character.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "CAP_PlayerCharacter.generated.h"

enum class EAbilityInputType : uint8;
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
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	/**		컴포넌트		**/
	UPROPERTY(VisibleDefaultsOnly, Category="Camera")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleDefaultsOnly, Category="Camera")
	class UCameraComponent* Camera;
	
	/**		입력			**/
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputMappingContext* GameplayIMC;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* MoveInputAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* DirFixInputAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TMap<EAbilityInputType, class UInputAction*> GameplayAbilityIAMap;

	void MoveInputHandle(const FInputActionValue& InputActionValue);
	void ToggleDirFixHandle();
	void AbilityInputHandle(const FInputActionValue& InputActionValue, EAbilityInputType AbilityInput);

	FVector GetMoveForwardVector() const;
	FVector GetMoveRightVector() const;
};
