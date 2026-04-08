// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/CAP_WeaponInstance.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

void UCAP_WeaponInstance::InitializeWeapon(UCAP_WeaponDataAsset* InWeaponDA)
{
	if (!InWeaponDA)
		return;

	WeaponDA = InWeaponDA;

	//등급에 따른 착용 가능한 스킬 수 가져오기
	int32 SkillsToGrant = 0;
	switch (WeaponDA->DefaultGrade)
	{
		case EItemGrade::Normal:		SkillsToGrant = 1;	break;
		case EItemGrade::Rare:		SkillsToGrant = 1;	break;
		case EItemGrade::Epic:		SkillsToGrant = 2;	break;
		case EItemGrade::Legendary:	SkillsToGrant = 2;	break;
	}

	// DA에 설정한 스킬을 랜덤으로 가져오기
	TArray<FWeaponSkillData> AvailableSkills = WeaponDA->ActiveAbilityArray;
	for (int32 i = 0; i < SkillsToGrant; ++i)
	{
		if (AvailableSkills.Num() == 0)
			break;

		int32 RandomIdx = FMath::RandRange(0, AvailableSkills.Num() - 1);

		GrantedActiveSkills.Add(AvailableSkills[RandomIdx]);
		// 중복 스킬 부여하지 않도록
		AvailableSkills.RemoveAt(RandomIdx);
	}
}
