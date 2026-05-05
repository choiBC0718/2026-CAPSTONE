// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_ItemDetailPanelWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "Interactables/Item/CAP_ItemInstance.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"

void UCAP_ItemDetailPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UCAP_ItemDetailPanelWidget::UpdateDetailInfo(UObject* ItemData, ESlotItemType ItemType)
{
	FeatureIconBox->ClearChildren();

	if (!ItemData)
	{
		Icon->SetVisibility(ESlateVisibility::Collapsed);
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
			if (UCAP_ItemDataBase* WeaponDA = WeaponInst->GetWeaponDA())
			{
				SetupUIContents(WeaponDA);
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
			if (UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA())
			{
				SetupUIContents(ItemDA);

				ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn());
				if (!Player || !Player->GetInventoryComponent())
					return;

				const TMap<FGameplayTag, FSynergyDataTable*>& SynergyCache = Player->GetInventoryComponent()->GetSynergyDataCache();

				TArray<FGameplayTag> Synergies = ItemDA->GetSynergyTags();
				for (const FGameplayTag& Tag : Synergies)
				{
					if (FSynergyDataTable* FoundRow = SynergyCache.FindRef(Tag))
					{
						AddFeatureIconToBox(FoundRow->SynergyIcon);
					}
				}
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

void UCAP_ItemDetailPanelWidget::SetupUIContents(class UCAP_ItemDataBase* ItemDA)
{
	if (!ItemDA)
		return;

	NameText->SetText(ItemDA->ItemName);
	GradeText->SetText(GetGradeText(ItemDA->ItemGrade));
	DescriptionText->SetText(ItemDA->ItemDescription);

	if (UTexture2D* LoadedIcon = ItemDA->ItemIcon.LoadSynchronous())
	{
		Icon->SetBrushFromTexture(LoadedIcon);
	}
}
