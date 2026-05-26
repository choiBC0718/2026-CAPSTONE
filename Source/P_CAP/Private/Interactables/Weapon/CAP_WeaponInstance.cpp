// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Weapon/CAP_WeaponInstance.h"

#include "Engine/AssetManager.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

void UCAP_WeaponInstance::Initialize(UCAP_ItemDataBase* InWeaponDA)
{
	if (!InWeaponDA)
		return;
	Super::Initialize(InWeaponDA);

	// 기본 공격 데이터 가져오기
	if (FWeaponSkillData* BasicAttackRow = GetWeaponDA()->BasicAbility.GetRow<FWeaponSkillData>(""))
	{
		BasicAttackData = *BasicAttackRow;
	}
	
	const int32 SkillsToGrant = GetSkillCountByGrade(GetCurrentGrade());
	
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
		if (!SkillData.AbilityMontage.IsNull())
			AssetsToLoad.AddUnique(SkillData.AbilityMontage.ToSoftObjectPath());
		if (!SkillData.SkillIcon.IsNull())
		{
			AssetsToLoad.AddUnique(SkillData.SkillIcon.ToSoftObjectPath());
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

FBuffDisplayData UCAP_WeaponInstance::GetBuffDisplayData(const FGameplayTag& EffectTag) const
{
	FBuffDisplayData BuffData;
	if (GetWeaponDA())
	{
		BuffData.Icon = GetWeaponDA()->ItemIcon;
	}
	if (EffectTag.IsValid())
	{
		for (const FWeaponSkillData& Skill : GetGrantedSkills())
		{
			if (Skill.CooldownTag == EffectTag)
			{
				BuffData.Icon = Skill.SkillIcon;
				break;
			}
		}
	}
	return BuffData;
}

bool UCAP_WeaponInstance::UpgradeWeapon()
{
	if (CurrentGrade>=EItemGrade::Legendary || !GetWeaponDA())
		return false;

	EItemGrade OldGrade = CurrentGrade;
	CurrentGrade = static_cast<EItemGrade>(static_cast<uint8>(CurrentGrade)+1);

	// 레어 -> 에픽 업그레이드 시 스킬 1개 더 받음
	if (OldGrade == EItemGrade::Rare && CurrentGrade == EItemGrade::Epic)
		TryAppendRandomNewSkill();
	
	return true;
}

int32 UCAP_WeaponInstance::GetSkillCountByGrade(EItemGrade Grade)
{
	switch (Grade)
	{
	case EItemGrade::Normal:
	case EItemGrade::Rare:
		return 1;
	case EItemGrade::Epic:
	case EItemGrade::Legendary:
		return 2;
	default:
		return 0;
	}
}

bool UCAP_WeaponInstance::TryAppendRandomNewSkill()
{
	if (!GetWeaponDA())
		return false;
	TArray<FDataTableRowHandle> AvailableHandles = GetWeaponDA()->ActiveAbilityArray;
	for (int32 i= AvailableHandles.Num()-1; i >= 0; --i)
	{
		if (FWeaponSkillData* SkillRow = AvailableHandles[i].GetRow<FWeaponSkillData>(""))
		{
			const bool bAlreadyHas = GrantedActiveSkills.ContainsByPredicate([SkillRow](const FWeaponSkillData& HasSkill)
			{
				return HasSkill.SkillName == SkillRow->SkillName;
			});

			if (bAlreadyHas)
				AvailableHandles.RemoveAt(i);
		}
	}
	if (AvailableHandles.Num() <= 0)
		return false;

	const int32 RandIdx = FMath::RandRange(0, AvailableHandles.Num() - 1);
	if (FWeaponSkillData* NewSkill = AvailableHandles[RandIdx].GetRow<FWeaponSkillData>(""))
	{
		GrantedActiveSkills.Add(*NewSkill);
		return true;
	}
	return false;
}
