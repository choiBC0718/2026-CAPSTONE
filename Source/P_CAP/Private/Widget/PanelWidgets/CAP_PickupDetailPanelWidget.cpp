// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_PickupDetailPanelWidget.h"

#include "Components/TextBlock.h"
#include "Items/Weapon/CAP_WeaponInstance.h"
#include "Widget/Common/CAP_ItemInteraction.h"


void UCAP_PickupDetailPanelWidget::UpdateDetailInfo(UObject* ItemData)
{
	Super::UpdateDetailInfo(ItemData);

	if (UCAP_WeaponInstance* WeaponInst = Cast<UCAP_WeaponInstance>(ItemData))
	{
		if (UCAP_WeaponDataAsset* WeaponDA = WeaponInst->GetWeaponDA())
		{
			ItemNameText->SetText(WeaponDA->WeaponName);
			ItemGradeText->SetText(GetGradeText(WeaponDA->DefaultGrade));
			ItemDescriptionText->SetText(WeaponDA->Description);

			for (const FWeaponSkillData SkillData : WeaponInst->GetGrantedSkills())
			{
				AddFeatureIconToBox(SkillData.SkillIcon);
			}
		}
	}
}

void UCAP_PickupDetailPanelWidget::UpdateInteractionUI(bool bVisible, UObject* ItemData, const FString& KeyName)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (bVisible)
	{
		UpdateDetailInfo(ItemData);
		if (InteractTextWidget)
		{
			InteractTextWidget->SetInteractKeyText(KeyName);
		}
	}
}

void UCAP_PickupDetailPanelWidget::UpdateInteractProgress(float Progress)
{
	if (InteractTextWidget)
		InteractTextWidget->UpdateInteractProgress(Progress);
}

