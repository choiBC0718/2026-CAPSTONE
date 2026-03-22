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
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void PickupWeapon(class UCAP_WeaponDataAsset* NewWeaponDA);
	void SwapWeapon();
	
	void SetNearbyInteractable(AActor* NewInteractable) {InteractableActor = NewInteractable;}
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
	void InteractInputHandle(const FInputActionValue& InputActionValue);

protected:
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	class UCAP_WeaponDataAsset* DefaultBasicWeapon;
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TArray<class UCAP_WeaponDataAsset*> EquippedWeapons;
	UPROPERTY()
	int32 CurrentWeaponIndex = 0;

	UPROPERTY()
	AActor* InteractableActor;
};
