// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "InputAction.h"
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
	void PickupWeapon(class UCAP_WeaponInstance* NewWeaponInstance);
	UFUNCTION()
	class UCAP_WeaponInstance* GetCurrentWeaponInstance() const;
	
	void SetNearbyInteractable(AActor* NewInteractable) {InteractableActor = NewInteractable;}
	void UpdateInteractUI(bool bVisible);
	void UpdateInteractProgress(float Progress);
	
private:
	/**		Components		**/
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	class USceneComponent* WeaponAttachPoint_R;
	UPROPERTY()
	class USkeletalMeshComponent* WeaponMesh_R;
	UPROPERTY(VisibleAnywhere, Category="Weapon")
	class USceneComponent* WeaponAttachPoint_L;
	UPROPERTY()
	class USkeletalMeshComponent* WeaponMesh_L;
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
	void InteractInputHandle(const FInputActionInstance& Instance);
	void SwapWeapon();

protected:
	/** 게임 시작 시 기본 무기 (주먹 무기로 설정) */
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	class UCAP_WeaponDataAsset* DefaultBasicWeapon;
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TArray<class UCAP_WeaponInstance*> EquippedWeapons;

	UPROPERTY()
	int32 CurrentWeaponIndex = 0;
	UPROPERTY()
	AActor* InteractableActor;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> CurrentWeaponAbilityHandles;
	
	void ApplyWeaponData(class UCAP_WeaponInstance* WeaponInstance);
	void ClearCurrentWeaponData();
};
