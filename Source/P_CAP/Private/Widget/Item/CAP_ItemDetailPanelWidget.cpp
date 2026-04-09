// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_ItemDetailPanelWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Items/Item/CAP_ItemInstance.h"
#include "Items/Weapon/CAP_WeaponInstance.h"

void UCAP_ItemDetailPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UCAP_ItemDetailPanelWidget::UpdateDetailInfo(UObject* ItemData, ESlotItemType ItemType)
{
	if (FeatureIconBox)
	{
		FeatureIconBox->ClearChildren();
	}

	if (!ItemData)
	{
		Icon->SetVisibility(ESlateVisibility::Hidden);
		NameText->SetText(FText::GetEmpty());
		GradeText->SetText(FText::GetEmpty());
		DescriptionText->SetText(FText::GetEmpty());
		return;
	}

	Icon->SetVisibility(ESlateVisibility::Visible);
	if (ItemType == ESlotItemType::Weapon)
	{
		if (UCAP_WeaponInstance* WeaponInst = Cast<UCAP_WeaponInstance>(ItemData))
		{
			if (UCAP_WeaponDataAsset* WeaponDA = WeaponInst->GetWeaponDA())
			{
				NameText->SetText(WeaponDA->WeaponName);
				GradeText->SetText(GetGradeText(WeaponDA->DefaultGrade));
				DescriptionText->SetText(WeaponDA->Description);

				if (UTexture2D* LoadedIcon = WeaponDA->WeaponIcon.LoadSynchronous())
				{
					Icon->SetBrushFromTexture(LoadedIcon);
				}
				for (const FWeaponSkillData SkillData : WeaponInst->GetGrantedSkills())
				{
					UImage* SkillIconImg = NewObject<UImage>(this);
					if (UTexture2D* SkillIcon = SkillData.SkillIcon.LoadSynchronous())
					{
						SkillIconImg->SetBrushFromTexture(SkillIcon);
						SkillIconImg->SetDesiredSizeOverride(FVector2D(100.f,100.f));
					}

					UHorizontalBoxSlot* HBoxSlot = FeatureIconBox->AddChildToHorizontalBox(SkillIconImg);
					if (HBoxSlot)
					{
						HBoxSlot->SetPadding(FMargin(5.f));
					}
				}
			}
		}
	}
	else if (ItemType == ESlotItemType::Item)
	{
		if (UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(ItemData))
		{
			if (UCAP_ItemDataAsset* ItemDA = ItemInst->GetItemDA())
			{
				NameText->SetText(ItemDA->ItemName);
				GradeText->SetText(GetGradeText(ItemDA->ItemGrade));
				DescriptionText->SetText(ItemDA->ItemDescription);

				if (UTexture2D* LoadedIcon = ItemDA->ItemIcon.LoadSynchronous())
				{
					Icon->SetBrushFromTexture(LoadedIcon);
				}

				//TODO: 시너지 시스템 기능 추가
			}
		}
	}
}

FText UCAP_ItemDetailPanelWidget::GetGradeText(EItemGrade Grade) const
{
	switch (Grade)
	{
	case EItemGrade::Normal:		return FText::FromString(TEXT("일반"));		break;
	case EItemGrade::Rare:			return FText::FromString(TEXT("레어"));		break;
	case EItemGrade::Epic:			return FText::FromString(TEXT("에픽"));		break;
	case EItemGrade::Legendary:		return FText::FromString(TEXT("레전더리"));	break;
	default:						return FText::GetEmpty();			break;
	}
}
