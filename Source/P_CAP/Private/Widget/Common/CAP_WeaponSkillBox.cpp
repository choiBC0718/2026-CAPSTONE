// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_WeaponSkillBox.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_WeaponComponent.h"
#include "Components/WrapBox.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"
#include "Widget/SlotWidgets/CAP_AbilitySlot.h"


void UCAP_WeaponSkillBox::NativeConstruct()
{
	Super::NativeConstruct();
	if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			// 무기 변경 델리게이트 연결
			WeaponComp->OnWeaponChanged.AddDynamic(this, &UCAP_WeaponSkillBox::HandleWeaponChanged);
			WeaponComp->OnWeaponSkillChanged.AddDynamic(this, &UCAP_WeaponSkillBox::HandleWeaponSkillChanged);
		}
	}
}

void UCAP_WeaponSkillBox::HandleWeaponChanged(class UCAP_WeaponInstance* MainWeaponInst, class UCAP_WeaponInstance* SubWeaponInst)
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

void UCAP_WeaponSkillBox::HandleWeaponSkillChanged(class UCAP_WeaponInstance* WeaponInst)
{
	ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	if (!Player)
		return;
	
	UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent();
	UCAP_WeaponInstance* MainWeapon = WeaponComp->GetCurrentWeaponInstance();
	UCAP_WeaponInstance* SubWeapon = nullptr;
	
	for (UCAP_WeaponInstance* Weapon : WeaponComp->GetEquippedWeapons())
	{
		if (Weapon && Weapon != MainWeapon)
		{
			SubWeapon = Weapon;
			break;
		}
	}
	HandleWeaponChanged(MainWeapon, SubWeapon);
}
