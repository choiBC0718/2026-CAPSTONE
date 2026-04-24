// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Engine/DataAsset.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_ItemDataAsset.generated.h"

UENUM(BlueprintType)
enum class EItemSkillActiveType : uint8
{
	Trigger			/** 특정 상황에 발동*/,
	AutoActivate	/** 쿨타임 돌 때마다 자동 발동*/,
};

USTRUCT(BlueprintType)
struct FItemEffectPayload
{
	GENERATED_BODY()

	// 도트데미지 GE / 디버프 GE 
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UGameplayEffect> MasterGEClass;

	// 도트 데미지인 경우 어떤 속성에 영향을 받을지
	UPROPERTY(EditDefaultsOnly)
	FGameplayAttribute BaseAttribute;
	// 도트데미지 : 속성의 몇퍼만큼 || 디버프 : 몇 초 동안
	UPROPERTY(EditDefaultsOnly)
	float Magnitude = 0.f;
	// 부여할 디버프 태그
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag DynamicTag;
};

USTRUCT(BlueprintType)
struct FItemSkillData
{
	GENERATED_BODY()

	// 아이템 스킬 발동 타입
	UPROPERTY(EditDefaultsOnly)
	EItemSkillActiveType ActiveType = EItemSkillActiveType::Trigger;
	// 어떤 태그로 Trigger를 받을 것인지
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "ActiveType == EItemSkillActiveType::Trigger", Categories="Ability.Event.ItemTrigger"))
	FGameplayTag TriggerEventTag;
	// 발동을 위한 트리거 개수
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="1", EditCondition = "ActiveType == EItemSkillActiveType::Trigger"))
	int32 RequiredTriggerCount =1;

	// 스킬이 주는 효과
	UPROPERTY(EditDefaultsOnly)
	TArray<FItemEffectPayload> ItemEffectPayloads;
	// 무언갈 소환하는 스킬인 경우 설정
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AActor> SpawnActorClass;
	// 아이템 스킬 쿨타임
	UPROPERTY(EditDefaultsOnly)
	float Cooldown = 1.f;
	// 쿨타임 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Ability.Cooldown.Item"))
	FGameplayTag CooldownTag;
	// 작게 보여줄 쿨타임 아이콘
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UTexture2D> CooldownIcon;
};

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
	// 아이템이 실행할 GA
	UPROPERTY(EditDefaultsOnly, Category="Item Effect")
	TArray<FItemSkillData> ItemSkills;
	
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
};