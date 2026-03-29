// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "UObject/NoExportTypes.h"
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

	const TArray<FWeaponSkillData>& GetGrantedSkills() const {return GrantedActiveSkills;}

private:
	UPROPERTY(VisibleAnywhere)
	UCAP_WeaponDataAsset* WeaponDA;
	UPROPERTY(VisibleAnywhere)
	TArray<FWeaponSkillData> GrantedActiveSkills;
	UPROPERTY(VisibleAnywhere)
	int32 UpgradeLevel =0;
};
