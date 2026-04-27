// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_WeaponDataAsset.h"
#include "GameplayEffect.h"
#include "Engine/DataAsset.h"
#include "CAP_AbilitySystemGenerics.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_AbilitySystemGenerics : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FORCEINLINE TSubclassOf<UGameplayEffect> GetFullStatEffect() const {return FullStatEffect;}
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDeathEffect() const {return DeathEffect;}
	FORCEINLINE const TArray<TSubclassOf<UGameplayEffect>>& GetInitialEffects() const {return InitialEffects;}
	FORCEINLINE const TArray<TSubclassOf<UGameplayAbility>>& GetPassiveAbilities() const {return PassiveAbilities;}

	FORCEINLINE const UDataTable* GetBaseStatDataTable() const {return BaseStatDataTable;}
	FORCEINLINE const UDataTable* GetWeaponStatDataTable() const {return WeaponStatDataTable;}
	
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetItemStatInfiniteEffect() const {return MasterStatInfiniteGE;}
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetCooldownEffect() const {return MasterCooldownGE;}

	TSubclassOf<UGameplayEffect> GetDamageGE(ESkillDamageType Type) const;
	TSubclassOf<UGameplayEffect> GetItemMasterGE(EItemExecutionType Type) const;
	
private:
	/**플레이어 & 몬스터 최초 스폰 시 HP 가득 채우는 Effect*/
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TSubclassOf<UGameplayEffect> FullStatEffect;
	/**플레이어 & 몬스터 사망 시 사망 태그 부여할 GE*/
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TSubclassOf<UGameplayEffect> DeathEffect;
	/**플레이어 & 몬스터 초기화 GE (시간당 HP 회복 / 태그 부여 등) */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TArray<TSubclassOf<UGameplayEffect>> InitialEffects;
	/**플레이어 & 몬스터 패시브 능력*/
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TArray<TSubclassOf<UGameplayAbility>> PassiveAbilities;

	UPROPERTY(EditDefaultsOnly, Category="Base Stats")
	UDataTable* BaseStatDataTable;
	UPROPERTY(EditDefaultsOnly, Category="Weapon Stats")
	UDataTable* WeaponStatDataTable;

	// 물리 데미지 GE - ExecCalc Physical 설정
	UPROPERTY(EditDefaultsOnly, Category="Skill Master GE")
	TSubclassOf<class UGameplayEffect> MasterPhysicalDamageGE;
	// 마법 데미지 GE - ExecCalc Magical 설정
	UPROPERTY(EditDefaultsOnly, Category="Skill Master GE")
	TSubclassOf<class UGameplayEffect> MasterMagicalDamageGE;
	// 아이템이 제공하는 보너스 스탯을 넣을 마스터 클래스
	UPROPERTY(EditDefaultsOnly, Category="Skill Master GE")
	TSubclassOf<class UGameplayEffect> MasterCooldownGE;
	
	// 아이템 착용 시, 아이템 자체의 효과 보너스 스탯을 Infinite로 넣을 마스터 클래스
	UPROPERTY(EditDefaultsOnly, Category="Item Master GE")
	TSubclassOf<class UGameplayEffect> MasterStatInfiniteGE;
	// 아이템의 스킬로 일시적 스탯 증가 - Has Duration
	UPROPERTY(EditDefaultsOnly, Category="Item Master GE")
	TSubclassOf<class UGameplayEffect> MasterStatDurationGE;
	// 아이템의 스킬 즉발 데미지 - Instant
	UPROPERTY(EditDefaultsOnly, Category="Item Master GE")
	TSubclassOf<class UGameplayEffect> MasterInstantDamageGE;
	// 아이템의 스킬 틱 데미지 + 상태이상용 - Has Duration
	UPROPERTY(EditDefaultsOnly, Category="Item Master GE")
	TSubclassOf<class UGameplayEffect> MasterDotDamageGE;
};
