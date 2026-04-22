// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_AbilityListView.h"

#include "Data/CAP_AbilitySlotData.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Items/Weapon/CAP_WeaponInstance.h"


void UCAP_AbilityListView::RefreshWeaponSkills(class UCAP_WeaponInstance* WeaponInstance)
{
	if (!WeaponInstance)
		return;
	ClearListItems();
	
	const TArray<FWeaponSkillData>& GrantedSkills = WeaponInstance->GetGrantedSkills();
	for (const FWeaponSkillData& SkillData : GrantedSkills)
	{
		UCAP_AbilitySlotData* SlotData = NewObject<UCAP_AbilitySlotData>(this);
		
		SlotData->SkillIcon			= SkillData.SkillIcon;
		SlotData->SkillName			= SkillData.SkillName;
		SlotData->SkillDescription	= SkillData.Description;
		SlotData->CooldownTag		= SkillData.CooldownTag;

		AddItem(SlotData);
	}
}
