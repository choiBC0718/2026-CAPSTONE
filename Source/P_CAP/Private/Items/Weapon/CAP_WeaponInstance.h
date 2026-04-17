// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Engine/StreamableManager.h"
#include "CAP_WeaponInstance.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_WeaponInstance : public UObject
{
	GENERATED_BODY()

public:
	void InitializeWeapon(UCAP_WeaponDataAsset* WeaponDA);

	UFUNCTION()
	UCAP_WeaponDataAsset* GetWeaponDA() const {return WeaponDA;}

	// 비동기 로딩 지시함수
	void LoadWeaponAssets(FStreamableDelegate OnLoaded);
	// 무기 드랍 시 메모리 비우도록
	void UnloadWeaponAssets();

	const FWeaponSkillData* GetBasicAttack() const {return &BasicAttackData;}
	const TArray<FWeaponSkillData>& GetGrantedSkills() const {return GrantedActiveSkills;}

private:
	UPROPERTY(VisibleAnywhere)
	UCAP_WeaponDataAsset* WeaponDA;
	
	UPROPERTY(VisibleAnywhere)
	int32 UpgradeLevel =0;

	// 캐시 데이터
	FWeaponSkillData BasicAttackData;
	TArray<FWeaponSkillData> GrantedActiveSkills;

	// 에셋을 메모리에 잡아둘 핸들
	TSharedPtr<FStreamableHandle> AssetLoadHandle;
};
