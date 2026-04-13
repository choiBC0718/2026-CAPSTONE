// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "Character/CAP_Character.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "Items/Item/CAP_InventoryComponent.h"
#include "Items/Weapon/CAP_WeaponComponent.h"
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
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	
	void UpdateInteractUI(bool bVisible);
	void UpdateInteractProgress(float Progress);
	
	UCAP_WeaponComponent* GetWeaponComponent() const {return WeaponComponent;}
	UCAP_InventoryComponent* GetInventoryComponent() const {return InventoryComponent;}
	
private:
	/**		Components		**/
	UPROPERTY(VisibleAnywhere, Category="View")
	class USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category="View")
	class UCameraComponent* Camera;
	UPROPERTY(EditAnywhere, Category="Weapon")
	class UCAP_WeaponComponent* WeaponComponent;
	UPROPERTY(EditAnywhere, Category="Inventory")
	class UCAP_InventoryComponent* InventoryComponent;
	UPROPERTY(VisibleAnywhere, Category="AI|Tracker")
	class UPlayerTrackerComponent* PlayerTracker;

	/**		Input			**/
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputMappingContext* GameplayIMC;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* MoveIA;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* SwapIA;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* InteractIA;

	UPROPERTY(EditDefaultsOnly, Category="Input|Ability")
	TMap<EAbilityInputID, class UInputAction*> AbilityInputActions;
	
	FVector GetMoveForwardDir();
	FVector GetMoveRightDir();
	void MoveInputHandle(const FInputActionValue& InputActionValue);
	void AbilityInputHandle(const FInputActionValue& InputActionValue, EAbilityInputID AbilityInputID);
	void InteractInputHandle(const FInputActionInstance& Instance);
	void SwapWeapon();
	
};
