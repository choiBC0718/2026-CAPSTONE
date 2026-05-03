// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/CAP_WeaponInstance.h"

#include "Engine/AssetManager.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

void UCAP_WeaponInstance::InitializeWeapon(UCAP_WeaponDataAsset* InWeaponDA)
{
	if (!InWeaponDA)
		return;

	ItemDA = InWeaponDA;

	// 기본 공격 데이터 가져오기
	if (FWeaponSkillData* BasicAttackRow = GetWeaponDA()->BasicAbility.GetRow<FWeaponSkillData>(""))
	{
		BasicAttackData = *BasicAttackRow;
	}
	
	//등급에 따른 착용 가능한 스킬 수 가져오기
	int32 SkillsToGrant = 0;
	switch (GetWeaponDA()->ItemGrade)
	{
		case EItemGrade::Normal:		SkillsToGrant = 1;	break;
		case EItemGrade::Rare:		SkillsToGrant = 1;	break;
		case EItemGrade::Epic:		SkillsToGrant = 2;	break;
		case EItemGrade::Legendary:	SkillsToGrant = 2;	break;
	}
	
	// DA에 설정한 DT스킬들을 랜덤으로 가져오기
	TArray<FDataTableRowHandle> AvailableHandles = GetWeaponDA()->ActiveAbilityArray;
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

UCAP_WeaponDataAsset* UCAP_WeaponInstance::GetWeaponDA() const
{
	return Cast<UCAP_WeaponDataAsset>(ItemDA);
}

void UCAP_WeaponInstance::LoadWeaponAssets(FStreamableDelegate OnLoaded)
{
	// 무기가 부여할 능력들의 주소 수집할 Array
	TArray<FSoftObjectPath> AssetsToLoad;

	for (const FWeaponVisualInfo& VisualInfo : GetWeaponDA()->WeaponVisualInfos)
	{
		if (!VisualInfo.WeaponMesh.IsNull())
			AssetsToLoad.AddUnique(VisualInfo.WeaponMesh.ToSoftObjectPath());
	}

	auto ExtractAssetsFromSkill = [&](const FWeaponSkillData& SkillData)
	{
		if (!SkillData.AbilityClass.IsNull())
			AssetsToLoad.AddUnique(SkillData.AbilityClass.ToSoftObjectPath());
		if (!SkillData.AbilityMontage.IsNull())
			AssetsToLoad.AddUnique(SkillData.AbilityMontage.ToSoftObjectPath());

		if (const FTargetingLogicData* TargetingData = SkillData.LogicData.GetPtr<FTargetingLogicData>())
		{
			if (!TargetingData->CastMontage.IsNull())
				AssetsToLoad.AddUnique(TargetingData->CastMontage.ToSoftObjectPath());
			if (!TargetingData->TargetActorClass.IsNull())
				AssetsToLoad.AddUnique(TargetingData->TargetActorClass.ToSoftObjectPath());
			if (!TargetingData->RangeIndicatorClass.IsNull())
				AssetsToLoad.AddUnique(TargetingData->RangeIndicatorClass.ToSoftObjectPath());
			if (!TargetingData->ProjectileClass.IsNull())
				AssetsToLoad.AddUnique(TargetingData->ProjectileClass.ToSoftObjectPath());
		}
		else if (const FProjectileLogicData* ProjData = SkillData.LogicData.GetPtr<FProjectileLogicData>())
		{
			if (!ProjData->ProjectileClass.IsNull())
				AssetsToLoad.AddUnique(ProjData->ProjectileClass.ToSoftObjectPath());
		}
	};
	
	ExtractAssetsFromSkill(BasicAttackData);
	for (const FWeaponSkillData& Skill : GrantedActiveSkills)
	{
		ExtractAssetsFromSkill(Skill);
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

void UCAP_WeaponInstance::SwapSkillOrder()
{
	if (GrantedActiveSkills.Num() >= 2)
		GrantedActiveSkills.Swap(0,1);
}
