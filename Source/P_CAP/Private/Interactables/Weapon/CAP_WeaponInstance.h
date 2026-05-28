// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Engine/StreamableManager.h"
#include "GameplayAbilitySpecHandle.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Interactables/Item/CAP_ItemInstance.h"
#include "CAP_WeaponInstance.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_WeaponInstance : public UCAP_ItemInstance
{
	GENERATED_BODY()

public:
	virtual void Initialize(UCAP_ItemDataBase* WeaponDA) override;

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;
	UFUNCTION()
	UCAP_WeaponDataAsset* GetWeaponDA() const;

	// 비동기 로딩 지시함수
	void LoadWeaponAssets(FStreamableDelegate OnLoaded);
	// 무기 드랍 시 메모리 비우도록
	void UnloadWeaponAssets();

	const FWeaponSkillData* GetBasicAttack() const {return &BasicAttackData;}
	const TArray<FWeaponSkillData>& GetGrantedSkills() const {return GrantedActiveSkills;}

	// 위젯에서 무기 슬롯에 포커스 주고 상호작용 키 입력 시, 스킬 순서 변경
	void SwapSkillOrder();
	virtual FBuffDisplayData GetBuffDisplayData(const FGameplayTag& EffectTag) const override;
	bool UpgradeWeapon();

	FWeaponSaveData CreateSaveData() const;
	void RestoreFromSaveData(const FWeaponSaveData& InData);
	
private:
	static int32 GetSkillCountByGrade(EItemGrade Grade);
	bool TryAppendRandomNewSkill();
	// 캐시 데이터
	FWeaponSkillData BasicAttackData;
	TArray<FWeaponSkillData> GrantedActiveSkills;

	// 에셋을 메모리에 잡아둘 핸들
	TSharedPtr<FStreamableHandle> AssetLoadHandle;
};
