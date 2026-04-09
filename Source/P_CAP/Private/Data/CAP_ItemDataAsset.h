// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_ItemDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 아이템 이름
	UPROPERTY(EditDefaultsOnly, Category="Data")
	FText ItemName;
	// 아이템 설명
	UPROPERTY(EditDefaultsOnly, Category="Data", meta=(MultiLine="true"))
	FText ItemDescription;
	// 아이템 아이콘
	UPROPERTY(EditDefaultsOnly, Category="Data")
	TSoftObjectPtr<class UTexture2D> ItemIcon;
	// 아이템 등급
	UPROPERTY(EditDefaultsOnly, Category="Data")
	EItemGrade ItemGrade = EItemGrade::Normal;
	// 아이템이 보유한 시너지 종류 태그
	UPROPERTY(EditDefaultsOnly, Category="Data", meta=(Categories="Synergy"))
	FGameplayTag SynergyTag1;
	UPROPERTY(EditDefaultsOnly, Category="Data", meta=(Categories="Synergy"))
	FGameplayTag SynergyTag2;
};
