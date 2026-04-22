// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "CAP_AbilitySlotData.generated.h"

/**
 * Ability List View에 WeaponSkillData의 데이터를 보내기 위해
 * UObject에 담아 전달
 */
UCLASS()
class UCAP_AbilitySlotData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<class UTexture2D> SkillIcon;
	UPROPERTY(BlueprintReadOnly)
	FName SkillName;
	UPROPERTY(BlueprintReadOnly)
	FText SkillDescription;
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag CooldownTag;
};
