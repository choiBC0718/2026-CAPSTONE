// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_WeaponSkillBox.h"

#include "Components/WrapBox.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Items/Weapon/CAP_WeaponInstance.h"
#include "Widget/SlotWidgets/CAP_AbilitySlot.h"


void UCAP_WeaponSkillBox::RefreshWeaponSkills(class UCAP_WeaponInstance* MainWeaponInst, class UCAP_WeaponInstance* SubWeaponInst)
{
	if (!AbilitySlotClass)
		return;

	if (MainAbilityWrapBox)
	{
		MainAbilityWrapBox->ClearChildren();
		if (!MainWeaponInst)
			return;
		
		const TArray<FWeaponSkillData>& GrantedSkills = MainWeaponInst->GetGrantedSkills();
		for (const FWeaponSkillData& SkillData : GrantedSkills)
		{
			UCAP_AbilitySlot* AbilitySlot = CreateWidget<UCAP_AbilitySlot>(this,AbilitySlotClass);
			if (AbilitySlot)
			{
				AbilitySlot->InitSlot(SkillData,true);
				MainAbilityWrapBox->AddChild(AbilitySlot);
			}
		}
	}

	if (SubAbilityWrapBox)
	{
		SubAbilityWrapBox->ClearChildren();
		if (!SubWeaponInst)
			return;

		const TArray<FWeaponSkillData>& SubSkills = SubWeaponInst->GetGrantedSkills();
		for (const FWeaponSkillData& SkillData : SubSkills)
		{
			if (UCAP_AbilitySlot* AbilitySlot = CreateWidget<UCAP_AbilitySlot>(this,AbilitySlotClass))
			{
				AbilitySlot->InitSlot(SkillData,false);
				SubAbilityWrapBox->AddChild(AbilitySlot);
			}
		}
	}
}
