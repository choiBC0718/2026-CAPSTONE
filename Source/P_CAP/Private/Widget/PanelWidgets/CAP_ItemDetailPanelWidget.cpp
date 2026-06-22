// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_ItemDetailPanelWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_SynergyDataAsset.h"
#include "Framework/Subsystem/CAP_SynergySubsystem.h"
#include "Interactables/Item/CAP_ItemInstance.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"
#include "Widget/Common/CAP_SkillToolTipWidget.h"
#include "Widget/Common/CAP_SynergyToolTipWidget.h"

void UCAP_ItemDetailPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UCAP_ItemDetailPanelWidget::UpdateDetailInfo(UObject* ItemData, ESlotItemType ItemType)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance())
		return;
	UCAP_SynergySubsystem* SynergySubsystem = World->GetGameInstance()->GetSubsystem<UCAP_SynergySubsystem>();
	if (!SynergySubsystem)
		return;
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
			SetupUIContents(WeaponInst);
			for (const FWeaponSkillData SkillData : WeaponInst->GetGrantedSkills())
			{
				AddSkillFeatureIcon(SkillData);
				//AddFeatureIconToBox(SkillData.SkillIcon);
			}
		}
	}
	else if (ItemType == ESlotItemType::Item)
	{
		if (UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(ItemData))
		{
			if (UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA())
			{
				SetupUIContents(ItemInst);

				TArray<FGameplayTag> Synergies = ItemDA->GetSynergyTags();
				for (const FGameplayTag& Tag : Synergies)
				{
					if (SynergySubsystem->SynergyMap.Contains(Tag))
					{
						if (UCAP_SynergyDataAsset* SynergyDA = SynergySubsystem->SynergyMap[Tag].LoadSynchronous())
							AddSynergyFeatureIcon(SynergyDA);
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

void UCAP_ItemDetailPanelWidget::SetupUIContents(class UCAP_ItemInstance* ItemInst)
{
	if (!ItemInst || !ItemInst->GetItemDA())
		return;

	NameText->SetText(ItemInst->GetItemDA()->ItemName);
	GradeText->SetText(GetGradeText(ItemInst->GetCurrentGrade()));
	DescriptionText->SetText(ItemInst->GetItemDA()->ItemDescription);

	if (UTexture2D* LoadedIcon = ItemInst->GetItemDA()->ItemIcon.LoadSynchronous())
	{
		Icon->SetBrushFromTexture(LoadedIcon);
	}
}

void UCAP_ItemDetailPanelWidget::AddSkillFeatureIcon(const struct FWeaponSkillData& SkillData)
{
	if (SkillData.SkillIcon.IsNull())
		return;
	UImage* FeatureIconImg = NewObject<UImage>(this);
	if (UTexture2D* LoadedIcon = SkillData.SkillIcon.LoadSynchronous())
	{
		FSlateBrush IconBrush;
		IconBrush.SetResourceObject(LoadedIcon);
		IconBrush.ImageSize = SkillSynergyIconSize;
		FeatureIconImg->SetBrush(IconBrush);
	}
	if (SkillTooltipClass)
	{
		if (UCAP_SkillToolTipWidget* SkillWidget = CreateWidget<UCAP_SkillToolTipWidget>(this, SkillTooltipClass))
		{
			SkillWidget->SetupToolTip(SkillData);
			FeatureIconImg->SetToolTip(SkillWidget);
		}
	}
	if (UHorizontalBoxSlot* HBoxSlot = FeatureIconBox->AddChildToHorizontalBox(FeatureIconImg))
	{
		HBoxSlot->SetPadding(FMargin(50.f, 0.f, 50.f, 0.f));
		HBoxSlot->SetHorizontalAlignment(HAlign_Fill);
		HBoxSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UCAP_ItemDetailPanelWidget::AddSynergyFeatureIcon(UCAP_SynergyDataAsset* SynergyDA)
{
	if (!SynergyDA || !SynergyDA->SynergyIcon) return;

	UImage* FeatureIconImg = NewObject<UImage>(this);
	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(SynergyDA->SynergyIcon.LoadSynchronous());
	IconBrush.ImageSize = SkillSynergyIconSize;
	FeatureIconImg->SetBrush(IconBrush);
	
	if (SynergyTooltipClass)
	{
		if (UCAP_SynergyToolTipWidget* SynergyWidget = CreateWidget<UCAP_SynergyToolTipWidget>(this, SynergyTooltipClass))
		{
			SynergyWidget->SetupToolTip(SynergyDA);
			FeatureIconImg->SetToolTip(SynergyWidget);
		}
	}
	if (UHorizontalBoxSlot* HBoxSlot = FeatureIconBox->AddChildToHorizontalBox(FeatureIconImg))
	{
		HBoxSlot->SetPadding(FMargin(50.f, 0.f, 50.f, 0.f));
		HBoxSlot->SetHorizontalAlignment(HAlign_Fill);
		HBoxSlot->SetVerticalAlignment(VAlign_Fill);
	}
}
