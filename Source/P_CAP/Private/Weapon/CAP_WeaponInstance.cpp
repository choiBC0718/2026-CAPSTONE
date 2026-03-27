// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CAP_WeaponInstance.h"

#include "GAS/Ability/CAP_GameplayAbility.h"

void UCAP_WeaponInstance::InitializeWeapon(UCAP_WeaponDataAsset* InWeaponDA)
{
	if (!InWeaponDA)
		return;

	WeaponDA = InWeaponDA;

	//등급에 따른 착용 가능한 스킬 수 가져오기
	int32 SkillsToGrant = 0;
	switch (WeaponDA->DefaultGrade)
	{
		case EWeaponGrade::Normal:		SkillsToGrant = 1;	break;
		case EWeaponGrade::Rare:		SkillsToGrant = 1;	break;
		case EWeaponGrade::Epic:		SkillsToGrant = 2;	break;
		case EWeaponGrade::Legendary:	SkillsToGrant = 2;	break;
	}

	// DA에 설정한 스킬을 랜덤으로 가져오기
	TArray<TSoftClassPtr<UCAP_GameplayAbility>> AvailableSkills = WeaponDA->ActiveSkillArray;
	for (int32 i = 0; i < SkillsToGrant; ++i)
	{
		if (AvailableSkills.Num() == 0)
			break;

		int32 RandomIdx = FMath::RandRange(0, AvailableSkills.Num() - 1);
		if (AvailableSkills[RandomIdx].LoadSynchronous())
		{
			GrantedActiveSkills.Add(AvailableSkills[RandomIdx].Get());
		}
		// 중복 스킬 부여하지 않도록
		AvailableSkills.RemoveAt(RandomIdx);
	}
}
