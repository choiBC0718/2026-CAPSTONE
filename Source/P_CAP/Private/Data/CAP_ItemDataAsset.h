// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "Items/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "CAP_ItemDataAsset.generated.h"


UCLASS(Abstract)
class UCAP_ItemDataBase : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// 아이템 이름
	UPROPERTY(EditDefaultsOnly, Category="Widget Data")
	FText ItemName;
	// 아이템 설명
	UPROPERTY(EditDefaultsOnly, Category="Widget Data", meta=(MultiLine="true"))
	FText ItemDescription;
	// 아이템 아이콘
	UPROPERTY(EditDefaultsOnly, Category="Widget Data")
	TSoftObjectPtr<class UTexture2D> ItemIcon;
	
	// 아이템 등급
	UPROPERTY(EditDefaultsOnly, Category="Data")
	EItemGrade ItemGrade = EItemGrade::Normal;

	virtual TArray<FGameplayTag> GetSynergyTags() const { return TArray<FGameplayTag>(); }
	virtual const TMap<FGameplayTag, float>& GetStatModifiers() const {static TMap<FGameplayTag, float> EmptyMap; return EmptyMap; }
};

/**
 * 
 */
UCLASS()
class UCAP_ItemDataAsset : public UCAP_ItemDataBase
{
	GENERATED_BODY()

public:
	// 아이템이 보유한 시너지 종류 태그
	UPROPERTY(EditDefaultsOnly, Category="Data", meta=(Categories="Synergy"))
	FGameplayTag SynergyTag1;
	UPROPERTY(EditDefaultsOnly, Category="Data", meta=(Categories="Synergy"))
	FGameplayTag SynergyTag2;

	// 아이템이 제공하는 보너스 스탯 <Key: 증가시킬 스탯 || Value: 증가값(Add)>
	UPROPERTY(EditDefaultsOnly, Category="Item Effect", meta=(ForceInlineRow, Categories="Data.ItemStat"))
	TMap<FGameplayTag, float> ItemStatModifiers;
	// 아이템 효과
	UPROPERTY(EditDefaultsOnly, Category="Item Effect")
	TArray<UCAP_ItemBehaviorBase*> ItemBehaviors;
	
	// 아이템 메쉬
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TSoftObjectPtr<class UStaticMesh> ItemMesh;
	// 메쉬 스케일 조절
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FVector MeshScale = FVector(1.f);

	
	virtual TArray<FGameplayTag> GetSynergyTags() const override
	{
		TArray<FGameplayTag> Tags;
		if (SynergyTag1.IsValid())	Tags.Add(SynergyTag1);
		if (SynergyTag2.IsValid())	Tags.Add(SynergyTag2);
		return Tags;
	}
	virtual const TMap<FGameplayTag, float>& GetStatModifiers() const override
	{
		return ItemStatModifiers;
	}

	// 소환수 클래스 캐싱 배열
	UPROPERTY()
	TArray<TSubclassOf<class AActor>> CachedSummonClasses;
#if WITH_EDITOR
	// 에디터에서 속성 변경 시 캐싱 데이터 갱신
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// 게임 실행 시 메모리에 로드될 때 호출되어 캐싱 데이터 갱신
	virtual void PostLoad() override;

private:
	// 캐싱 로직
	void UpdateCachedData();
};