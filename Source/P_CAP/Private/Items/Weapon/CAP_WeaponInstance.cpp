// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/CAP_WeaponInstance.h"

#include "Engine/AssetManager.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

void UCAP_WeaponInstance::InitializeWeapon(UCAP_WeaponDataAsset* InWeaponDA)
{
	if (!InWeaponDA)
		return;

	WeaponDA = InWeaponDA;

	// 기본 공격 데이터 가져오기
	if (FWeaponSkillData* BasicAttackRow = WeaponDA->BasicAbility.GetRow<FWeaponSkillData>(""))
	{
		BasicAttackData = *BasicAttackRow;
	}
	
	//등급에 따른 착용 가능한 스킬 수 가져오기
	int32 SkillsToGrant = 0;
	switch (WeaponDA->DefaultGrade)
	{
		case EItemGrade::Normal:		SkillsToGrant = 1;	break;
		case EItemGrade::Rare:		SkillsToGrant = 1;	break;
		case EItemGrade::Epic:		SkillsToGrant = 2;	break;
		case EItemGrade::Legendary:	SkillsToGrant = 2;	break;
	}
	
	// DA에 설정한 DT스킬들을 랜덤으로 가져오기
	TArray<FDataTableRowHandle> AvailableHandles = WeaponDA->ActiveAbilityArray;
	for (int32 i = 0; i < SkillsToGrant; ++i)
	{
		if (AvailableHandles.Num() == 0)
			break;

		int32 RandomIdx = FMath::RandRange(0, AvailableHandles.Num() - 1);

		// 가져온 행들을 뽑아
		if (FWeaponSkillData* SkillRow = AvailableHandles[RandomIdx].GetRow<FWeaponSkillData>(""))
		{	// 캐싱
			GrantedActiveSkills.Add(*SkillRow);
		}
		// 중복 스킬 부여하지 않도록
		AvailableHandles.RemoveAt(RandomIdx);
	}
}

void UCAP_WeaponInstance::LoadWeaponAssets(FStreamableDelegate OnLoaded)
{
	// 무기가 부여할 능력들의 주소 수집할 Array
	TArray<FSoftObjectPath> AssetsToLoad;

	// 기본 공격 에셋의 주소
	if (!BasicAttackData.AbilityClass.IsNull())
		AssetsToLoad.AddUnique(BasicAttackData.AbilityClass.ToSoftObjectPath());
	if (!BasicAttackData.AbilityMontage.IsNull())
		AssetsToLoad.AddUnique(BasicAttackData.AbilityMontage.ToSoftObjectPath());
	if (!BasicAttackData.SkillDamageTypeEffect.IsNull())
		AssetsToLoad.AddUnique(BasicAttackData.SkillDamageTypeEffect.ToSoftObjectPath());
	if (!BasicAttackData.ProjectileClass.IsNull())
		AssetsToLoad.AddUnique(BasicAttackData.ProjectileClass.ToSoftObjectPath());
	if (!BasicAttackData.CastMontage.IsNull())
		AssetsToLoad.AddUnique(BasicAttackData.CastMontage.ToSoftObjectPath());
	if (!BasicAttackData.TargetActorClass.IsNull())
		AssetsToLoad.AddUnique(BasicAttackData.TargetActorClass.ToSoftObjectPath());
	if (!BasicAttackData.RangeIndicatorClass.IsNull())
		AssetsToLoad.AddUnique(BasicAttackData.RangeIndicatorClass.ToSoftObjectPath());

	
	for (const FWeaponSkillData& Skill : GrantedActiveSkills)
	{	// 부여받은 스킬 에셋의 주소
		if (!Skill.AbilityClass.IsNull())
			AssetsToLoad.AddUnique(Skill.AbilityClass.ToSoftObjectPath());
		if (!Skill.AbilityMontage.IsNull())
			AssetsToLoad.AddUnique(Skill.AbilityMontage.ToSoftObjectPath());
		if (!Skill.SkillDamageTypeEffect.IsNull())
			AssetsToLoad.AddUnique(Skill.SkillDamageTypeEffect.ToSoftObjectPath());
		if (!Skill.ProjectileClass.IsNull())
			AssetsToLoad.AddUnique(Skill.ProjectileClass.ToSoftObjectPath());
		if (!Skill.CastMontage.IsNull())
			AssetsToLoad.AddUnique(Skill.CastMontage.ToSoftObjectPath());
		if (!Skill.TargetActorClass.IsNull())
			AssetsToLoad.AddUnique(Skill.TargetActorClass.ToSoftObjectPath());
		if (!Skill.RangeIndicatorClass.IsNull())
			AssetsToLoad.AddUnique(Skill.RangeIndicatorClass.ToSoftObjectPath());
	}

	// 수집한 주소들에 핸들 붙여주기
	if (AssetsToLoad.Num() > 0)
	{
		UAssetManager& AssetManager = UAssetManager::Get();
		AssetLoadHandle = AssetManager.GetStreamableManager().RequestAsyncLoad(AssetsToLoad, OnLoaded);
	}
	else
	{
		OnLoaded.ExecuteIfBound();
	}
}

void UCAP_WeaponInstance::UnloadWeaponAssets()
{
	if (AssetLoadHandle.IsValid())
	{	// 메모리 해제
		AssetLoadHandle->ReleaseHandle();
		AssetLoadHandle.Reset();
	}
}
