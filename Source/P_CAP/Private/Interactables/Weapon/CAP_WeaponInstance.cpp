// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Weapon/CAP_WeaponInstance.h"

#include "Engine/AssetManager.h"
#include "GameplayAbilitySpec.h"
#include "GAS/Ability/Flow/GA_FlowBase.h"
#include "GAS/Ability/Payload/GA_PayloadBase.h"
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
		default:										break;
	}
	CurrentGrade = GetWeaponDA()->ItemGrade;
	
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
	{
		TArray<FDataTableRowHandle> AvailableHandles = GetWeaponDA()->ActiveAbilityArray;
		for (int32 i= AvailableHandles.Num()-1; i >= 0; --i)
		{
			if (FWeaponSkillData* SkillRow = AvailableHandles[i].GetRow<FWeaponSkillData>(""))
			{
				// 이미 가진 스킬은 추가될 스킬에서 제외
				bool bAlreadyHas = false;
				for (const FWeaponSkillData& HasSkill : GrantedActiveSkills)
				{
					if (HasSkill.SkillName == SkillRow->SkillName)
					{
						bAlreadyHas = true;
						break;
					}
				}
				if (bAlreadyHas)
					AvailableHandles.RemoveAt(i);
			}
		}
		if (AvailableHandles.Num() > 0)
		{
			int32 RandIdx = FMath::RandRange(0, AvailableHandles.Num() - 1);
			if (FWeaponSkillData* NewSkill = AvailableHandles[RandIdx].GetRow<FWeaponSkillData>(""))
			{
				GrantedActiveSkills.Add(*NewSkill);
				if (UCAP_AbilitySystemComponent* ASC = GetCachedASC())
				{
					int32 InputID = static_cast<int32>(EAbilityInputID::Skill1) + GrantedActiveSkills.Num() - 1;

					if (NewSkill->InputAbilityClass)
					{
						FGameplayAbilitySpec Spec(NewSkill->InputAbilityClass,1,InputID,this);
						GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
					}
					for (TSubclassOf<UGA_PayloadBase> PayloadClass : NewSkill->PayloadAbilityClass)
					{
						if (PayloadClass)
						{
							FGameplayAbilitySpec Spec(PayloadClass, 1, InputID+100,this);
							GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
						}
					}
				}
			}
		}
	}
	return true;
}
