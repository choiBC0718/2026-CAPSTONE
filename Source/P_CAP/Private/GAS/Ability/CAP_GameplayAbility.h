// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "CAP_GameplayAbility.generated.h"

/**
 * GA 클래스 최상위 부모 : 헬퍼 함수만 존재
 * 흐름 + 결과물을 합쳐 한 개의 Ability로 제작
 * 흐름 = 입력 방식 (1회성, Combo, Holding, Charing)
 * 결과물 = 애니메이션 몽타주의 노티파이의 태그를 트리거로 실행되는 GA
 */
UCLASS()
class UCAP_GameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCAP_GameplayAbility();
	
protected:
	UAnimInstance* GetOwnerAnimInstance() const;
	class ACAP_PlayerCharacter* GetPlayerCharacterFromActorInfo() const;
	const struct FWeaponSkillData* GetSkillDataFromContext(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;

	FVector GetMuzzleSocketLocation(FName SocketName);
	void SendGameplayCueEvent(FHitResult HitResult, const struct FWeaponSkillData* InSkillData);
	void ApplyStatusEffectsToTarget(AActor* TargetActor, const TArray<EStatusEffectType>& Effects);
	
	TSubclassOf<UGameplayEffect> GetDamageGE() const;
	
	bool IsBasicAttack() const;
	void BroadcastTriggerEvent(FGameplayTag EventTag, FGameplayAbilityTargetDataHandle TargetData = FGameplayAbilityTargetDataHandle(), float EventMagnitude = 1.f,const UObject* OptionalObject = nullptr) const;
};
