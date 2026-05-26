// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayTagContainer.h"
#include "CAP_UIBuffListTypes.generated.h"


UENUM(BlueprintType)
enum class EBuffSourceType :uint8
{
	ASC_Effect,
	Item_Effect,
};

USTRUCT(BlueprintType)
struct FBuffSlotID
{
	GENERATED_BODY()

	UPROPERTY()
	EBuffSourceType SourceType = EBuffSourceType::Item_Effect;
	UPROPERTY()
	FActiveGameplayEffectHandle ActiveGEHandle;
	UPROPERTY()
	TObjectPtr<class UCAP_ItemInstance> ItemInst = nullptr;
	UPROPERTY()
	FGameplayTag ItemDynamicTag;
	
	bool operator==(const FBuffSlotID& Other) const
	{
		if (SourceType != Other.SourceType)	return false;
		if (SourceType == EBuffSourceType::ASC_Effect)	return ActiveGEHandle==Other.ActiveGEHandle;
		return (ItemInst == Other.ItemInst) && (ItemDynamicTag == Other.ItemDynamicTag);
	}
};

USTRUCT(BlueprintType)
struct FBuffUIData
{
	GENERATED_BODY()

	UPROPERTY()
	TSoftObjectPtr<UTexture2D> Icon;
	UPROPERTY()
	int32 Stacks=0;
	
	UPROPERTY()
	float MaxCooldown =0.f;
	UPROPERTY()
	float RemainingCooldown=0.f;
	
	UPROPERTY()
	float MaxDuration=0.f;
	UPROPERTY()
	float RemainingDuration =0.f;
	
	UPROPERTY()
	bool bIsDebuff=false;
};