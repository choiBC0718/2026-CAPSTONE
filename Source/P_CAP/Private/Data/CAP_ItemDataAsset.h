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
	Trigger,
	AutoActivate,
};

UENUM(BlueprintType)
enum class EItemExecutionType : uint8
{
	Instant_Damage	UMETA(DisplayName = "Instant Damage"),
	DotDamage		UMETA(DisplayName = "Dot Damage"),
	Buff_Self		UMETA(DisplayName = "Self Buff"),
	Debuff_Target	UMETA(DisplayName = "Target Debuff"),
	Destroy_Item	UMETA(DisplayName = "Item Destroy"),
	Upgrade_Item	UMETA(DisplayName = "Item Upgrade"),
	Custom_Ability	UMETA(DisplayName = "Custom Ability"),
};

USTRUCT(BlueprintType)
struct FItemEffectPayload
{
	GENERATED_BODY()
	// 아이템의 효과가 어떻게 작용할지
	UPROPERTY(EditDefaultsOnly, Category="Action")
	EItemExecutionType ExecutionType = EItemExecutionType::Instant_Damage;
	
	// 영향받을 스탯의 종류 - 선택 사항
	UPROPERTY(EditDefaultsOnly, Category="Value Setting", meta=(EditCondition="ExecutionType != EItemExecutionType::Destroy_Item || ExecutionType != EItemExecutionType::Upgrade_Item || ExecutionType != EItemExecutionType::Custom_Ability", EditConditionHides))
	FGameplayAttribute ScaleAttribute;
	// 기본 고정 값 (최종 값 = BaseValue + Attribute*Magnitude)
	UPROPERTY(EditDefaultsOnly, Category="Value Setting", meta=(EditCondition="ExecutionType != EItemExecutionType::Destroy_Item || ExecutionType != EItemExecutionType::Upgrade_Item || ExecutionType != EItemExecutionType::Custom_Ability", EditConditionHides))
	float BaseValue = 0.f;
	// 스탯의 계수 (0.1 = 10%)
	UPROPERTY(EditDefaultsOnly, Category="Value Setting", meta=(EditCondition="ExecutionType != EItemExecutionType::Destroy_Item || ExecutionType != EItemExecutionType::Upgrade_Item || ExecutionType != EItemExecutionType::Custom_Ability", EditConditionHides))
	float Magnitude = 0.f;
	
	// 어떤 스탯을 올리고 내릴지
	UPROPERTY(EditDefaultsOnly, Category="Buff Setting", meta=(Categories="Data.ItemStat",EditCondition="ExecutionType == EItemExecutionType::Buff_Self || ExecutionType == EItemExecutionType::Debuff_Target", EditConditionHides))
	FGameplayTag TargetStatTag;
	// 지속시간
	UPROPERTY(EditDefaultsOnly, Category="Buff Setting", meta=(EditCondition="ExecutionType == EItemExecutionType::Buff_Self || ExecutionType == EItemExecutionType::Debuff_Target || ExecutionType == EItemExecutionType::DotDamage", EditConditionHides))
	float Duration = 0.f;
	// 최대 몇 스택까지 중첩가능한지
	UPROPERTY(EditDefaultsOnly, Category="Buff Setting", meta=(EditCondition="ExecutionType == EItemExecutionType::Buff_Self || ExecutionType == EItemExecutionType::Debuff_Target || ExecutionType == EItemExecutionType::DotDamage", EditConditionHides))
	int32 MaxStackCount = 1;

	// 효과 발동 확률
	UPROPERTY(EditDefaultsOnly, Category="Common", meta=(ClampMin="0.0", ClampMax="100.0"))
	float TriggerChance = 100.f;
	// 스택 확인 중첩되는 태그
	UPROPERTY(EditDefaultsOnly, Category="Common", meta=(EditCondition="ExecutionType != EItemExecutionType::Destroy_Item || ExecutionType != EItemExecutionType::Upgrade_Item || ExecutionType != EItemExecutionType::Custom_Ability", EditConditionHides))
	FGameplayTag DynamicTag;
	// 특수 Ability
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta=(EditCondition="ExecutionType==EItemExecutionType::Custom_Ability",EditConditionHides))
	TSubclassOf<class UGameplayAbility> CustomAbilityClass;
};

USTRUCT(BlueprintType)
struct FItemSkillData
{
	GENERATED_BODY()

	// 아이템 스킬 발동 타입
	UPROPERTY(EditDefaultsOnly, Category="Logic")
	EItemSkillActiveType ActiveType = EItemSkillActiveType::Trigger;
	// 어떤 태그로 Trigger를 받을 것인지
	UPROPERTY(EditDefaultsOnly, Category="Logic", meta = (EditCondition = "ActiveType == EItemSkillActiveType::Trigger", Categories="Item.Trigger",EditConditionHides))
	FGameplayTag TriggerEventTag;
	// 발동을 위한 트리거 개수
	UPROPERTY(EditDefaultsOnly, Category="Logic", meta=(ClampMin="1", EditCondition = "ActiveType == EItemSkillActiveType::Trigger",EditConditionHides))
	int32 RequiredTriggerCount =1;

	// 스킬이 주는 효과
	UPROPERTY(EditDefaultsOnly, Category="Effect")
	TArray<FItemEffectPayload> ItemEffectPayloads;
	// 아이템 스킬 쿨타임
	UPROPERTY(EditDefaultsOnly, Category="Data")
	float Cooldown = 1.f;
	// 쿨타임 태그
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Cooldown"), Category="Data")
	FGameplayTag CooldownTag;
	// 무언갈 소환하는 스킬인 경우 설정
	UPROPERTY(EditDefaultsOnly, Category="Special")
	TSubclassOf<class AActor> SpawnActorClass;
	// 작게 보여줄 쿨타임 아이콘
	UPROPERTY(EditDefaultsOnly,Category="UI")
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