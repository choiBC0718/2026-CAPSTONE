// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	FORCEINLINE TArray<TSubclassOf<UGameplayEffect>> GetInitialEffects() const {return InitialEffects;}
	FORCEINLINE TArray<TSubclassOf<UGameplayAbility>> GetPassiveAbilities() const {return PassiveAbilities;}

	FORCEINLINE const UDataTable* GetBaseStatDataTable() const {return BaseStatDataTable;}
	FORCEINLINE const UDataTable* GetWeaponStatDataTable() const {return WeaponStatDataTable;}
	
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetItemStatEffectClass() const {return MasterStatEffectClass;}
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetCooldownEffectClass() const {return MasterCooldownEffectClass;}
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

	// 아이템이 제공하는 보너스 스탯을 넣을 마스터 클래스
	UPROPERTY(EditDefaultsOnly, Category="Item Effect")
	TSubclassOf<class UGameplayEffect> MasterStatEffectClass;
	// 아이템이 제공하는 보너스 스탯을 넣을 마스터 클래스
	UPROPERTY(EditDefaultsOnly, Category="Ability Effect")
	TSubclassOf<class UGameplayEffect> MasterCooldownEffectClass;
};
