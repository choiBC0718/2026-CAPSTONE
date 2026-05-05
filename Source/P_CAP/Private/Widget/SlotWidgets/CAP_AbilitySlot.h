// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Abilities/GameplayAbility.h"
#include "CAP_AbilitySlot.generated.h"

/**
 * 스킬의 아이콘, 쿨타임 나타내는 슬롯 -> AbilityListView를 통해 리스트 형식으로 사용
 */
UCLASS()
class UCAP_AbilitySlot : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void InitSlot(const FWeaponSkillData& SkillData, bool bIsActive = true);
	
private:
	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	float CooldownUpdateInterval = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName IconMaterialParamName = "Icon";
	
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FName CooldownPercentParamName = "CooldownPercent";
	
	UPROPERTY(meta = (BindWidget))
	class UImage* Icon;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownDurationText;

	UPROPERTY()
	FGameplayTag CooldownTag;
	UPROPERTY()
	class UAbilitySystemComponent* OwnerASC;

	void AbilityCommitted(UGameplayAbility* Ability);
	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);
	void CooldownFinished();
	void UpdateCooldown();

	float CachedCooldownDuration;
	float CachedCooldownTimeRemaining;
	
	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	FNumberFormattingOptions WholeNumberFormattingOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOptions;

	void CheckCurrentCooldown();

	UPROPERTY(EditDefaultsOnly)
	FVector2D SubSkillSize = FVector2D(0.f, 0.f);
	UPROPERTY(EditDefaultsOnly)
	FLinearColor SubSkillColor = FLinearColor(1.f,1.f,1.f,1.f);
};
