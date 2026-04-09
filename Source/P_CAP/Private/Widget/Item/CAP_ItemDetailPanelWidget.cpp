// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Item/CAP_ItemDetailPanelWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_SynergyTypes.h"
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
					AddFeatureIconToBox(SkillData.SkillIcon);
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

				if (!SynergyDataTable)
					return;

				TArray<FSynergyDataTable*> AllSynergies;
				SynergyDataTable->GetAllRows<FSynergyDataTable>("",AllSynergies);
				auto UpdateSynergyUI = [&](FGameplayTag Tag)
				{
					if (!Tag.IsValid())	return;
					for (FSynergyDataTable* Row : AllSynergies)
					{
						if (Row && Row->SynergyTag == Tag)
						{
							AddFeatureIconToBox(Row->SynergyIcon);
							break;
						}
					}
				};
				UpdateSynergyUI(ItemDA->SynergyTag1);
				UpdateSynergyUI(ItemDA->SynergyTag2);
			}
		}
	}
}

FText UCAP_ItemDetailPanelWidget::GetGradeText(EItemGrade Grade) const
{
	switch (Grade)
	{
	case EItemGrade::Normal:		return FText::FromString(TEXT("일반"));
	case EItemGrade::Rare:			return FText::FromString(TEXT("레어"));
	case EItemGrade::Epic:			return FText::FromString(TEXT("에픽"));
	case EItemGrade::Legendary:		return FText::FromString(TEXT("레전더리"));
	default:						return FText::GetEmpty();
	}
}

void UCAP_ItemDetailPanelWidget::AddFeatureIconToBox(TSoftObjectPtr<class UTexture2D> IconPtr)
{
	if (IconPtr.IsNull())
		return;

	UImage* FeatureIconImg = NewObject<UImage>(this);
	if (UTexture2D* LoadedIcon = IconPtr.LoadSynchronous())
	{
		FSlateBrush IconBrush;
		IconBrush.SetResourceObject(LoadedIcon);
		IconBrush.ImageSize = SkillSynergyIconSize;
		FeatureIconImg->SetBrush(IconBrush);
	}
	if (UHorizontalBoxSlot* HBoxSlot = FeatureIconBox->AddChildToHorizontalBox(FeatureIconImg))
	{
		HBoxSlot->SetPadding(FMargin(50.f, 0.f, 50.f, 0.f));
		HBoxSlot->SetHorizontalAlignment(HAlign_Fill);
		HBoxSlot->SetVerticalAlignment(VAlign_Fill);
	}
}
