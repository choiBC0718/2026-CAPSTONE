// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"
#include "CAP_WeaponSwapWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_WeaponSwapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	class UImage* CurrentWeaponImg;
	UPROPERTY(meta = (BindWidget))
	class UImage* StandbyWeaponImg;
	UPROPERTY(meta = (BindWidget))
	class UImage* SwapKeyImg;

private:
	UPROPERTY()
	class UAbilitySystemComponent* OwnerASC;
	
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName IconMaterialParamName = "Icon";
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName CooldownPercentParamName = "CooldownPercent";

	FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag("Ability.Cooldown.SwapWeapon");

	float CachedCooldownDuration;
	float CachedCooldownTimeRemaining;
	
	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;
	
	UFUNCTION()
	void HandleWeaponSwaped(class UCAP_WeaponInstance* NewWeaponInstance, class UCAP_WeaponInstance* OldWeaponInstance);
	void AbilityCommitted(UGameplayAbility* GameplayAbility);
	void OnCooldownTagChanged(FGameplayTag GameplayTag, int NewCount);

	void CheckCurrentCooldown();
	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);
	void CooldownFinished();
	void UpdateCooldown();

	void SetSwapKeyImg();
};
