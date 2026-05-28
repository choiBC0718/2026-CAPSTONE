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
	FORCEINLINE TSubclassOf<UGameplayEffect> GetOverrideEffect() const {return OverrideEffect;}
	FORCEINLINE const TArray<TSubclassOf<UGameplayEffect>>& GetInitialEffects() const {return InitialEffects;}
	FORCEINLINE const TArray<TSubclassOf<UGameplayAbility>>& GetPassiveAbilities() const {return PassiveAbilities;}

	FORCEINLINE const UDataTable* GetBaseStatDataTable() const {return BaseStatDataTable;}
	FORCEINLINE const UDataTable* GetWeaponStatDataTable() const {return WeaponStatDataTable;}

	FORCEINLINE TSubclassOf<class UGameplayEffect> GetItemMarkGE() const {return MasterMarkGE;}
	FORCEINLINE TSubclassOf<class UGameplayEffect> GetCooldownEffect() const {return MasterCooldownGE;}

	TSubclassOf<UGameplayEffect> GetInstantDamageGE(ESkillDamageType Type) const;
	TSubclassOf<UGameplayEffect> GetDurationDamageGE(ESkillDamageType Type) const;
	TSubclassOf<UGameplayEffect> GetStatGE(bool bIsInfinite, bool bIsMultiplier) const;

	static FName GetRowNameFromGrade(EItemGrade Grade);
private:
	/**플레이어 & 몬스터 최초 스폰 시 HP 가득 채우는 Effect*/
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TSubclassOf<UGameplayEffect> FullStatEffect;
	/**플레이어 & 몬스터 사망 시 사망 태그 부여할 GE*/
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TSubclassOf<UGameplayEffect> DeathEffect;
	/**스테이지 이동 시 초기화 되지 않도록 플레이어 스탯 오버라이드 할 GE */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effect")
	TSubclassOf<UGameplayEffect> OverrideEffect;
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
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Instant Damage Master GE")
	TSubclassOf<class UGameplayEffect> MasterPhysicalInstantDamageGE;
	// 마법 데미지 GE - ExecCalc Magical 설정
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Instant Damage Master GE")
	TSubclassOf<class UGameplayEffect> MasterMagicalInstantDamageGE;
	// Duration 물리 데미지 GE - ExecCalc Physical 설정
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Duration Damage Master GE")
	TSubclassOf<class UGameplayEffect> MasterPhysicalDurationDamageGE;
	// Duration 마법 데미지 GE - ExecCalc Magical 설정
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Duration Damage Master GE")
	TSubclassOf<class UGameplayEffect> MasterMagicalDurationDamageGE;
	// 쿨타임 마스터 GE
	UPROPERTY(EditDefaultsOnly, Category="Master GE")
	TSubclassOf<class UGameplayEffect> MasterCooldownGE;
	
	// 합연산 스탯 증가 - Infinite
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Stat GE")
	TSubclassOf<class UGameplayEffect> MasterStatInfiniteAddGE;
	// 합연산 일시적 스탯 증가 - Has Duration
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Stat GE")
	TSubclassOf<class UGameplayEffect> MasterStatDurationAddGE;
	// 곱연산 스탯 증가 - Infinite
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Stat GE")
	TSubclassOf<class UGameplayEffect> MasterStatInfiniteMulGE;
	// 곱연산 일시적 스탯 증가 - Has Duration
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Stat GE")
	TSubclassOf<class UGameplayEffect> MasterStatDurationMulGE;
	// 아이템 효과를 부여할때 어떤 아이템이 어떤 효과를 일으켰는지 구분하기 위한 GE
	UPROPERTY(EditDefaultsOnly, Category="Master GE|Item Master GE")
	TSubclassOf<class UGameplayEffect> MasterMarkGE;
};
