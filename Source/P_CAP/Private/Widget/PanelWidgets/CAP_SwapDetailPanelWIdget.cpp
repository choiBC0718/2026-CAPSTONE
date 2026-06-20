// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_SwapDetailPanelWIdget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_SynergyDataAsset.h"
#include "Framework/Subsystem/CAP_SynergySubsystem.h"
#include "Interactables/Item/CAP_ItemInstance.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"
#include "Widget/Common/CAP_SynergyToolTipWidget.h"

void UCAP_SwapDetailPanelWIdget::UpdateDetailInfo(UObject* ItemData)
{
	UWorld* World = GetWorld();
	if (!World || !World->GetGameInstance())
		return;
	UCAP_SynergySubsystem* SynergySubsystem = World->GetGameInstance()->GetSubsystem<UCAP_SynergySubsystem>();
	if (!SynergySubsystem)
		return;
	
	if (FeatureIconBox)
	{
		FeatureIconBox->ClearChildren();
	}

	if (!ItemData)
	{
		ItemNameText->SetText(FText::GetEmpty());
		ItemGradeText->SetText(FText::GetEmpty());
		ItemDescriptionText->SetText(FText::GetEmpty());
		return;
	}

	UCAP_WeaponInstance* WeaponInst = Cast<UCAP_WeaponInstance>(ItemData);
	UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(ItemData);
	
	UCAP_ItemDataBase* BaseItem = nullptr;
	if (WeaponInst)
	{
		BaseItem = WeaponInst->GetWeaponDA();
		ItemGradeText->SetText(GetGradeText(WeaponInst->GetCurrentGrade()));
	}
	else if (ItemInst)
	{
		BaseItem = ItemInst->GetItemDA();
		ItemGradeText->SetText(GetGradeText(ItemInst->GetCurrentGrade()));
	}
	
	if (BaseItem)
	{
		ItemNameText->SetText(BaseItem->ItemName);
		ItemDescriptionText->SetText(BaseItem->ItemDescription);
	}

	if (WeaponInst)
	{
		for (const FWeaponSkillData& SkillData : WeaponInst->GetGrantedSkills())
		{
			AddFeatureIconToBox(SkillData.SkillIcon);
		}
	}
	else if (ItemInst)
	{
		if (UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA())
		{
			for (const FGameplayTag& Tag : ItemDA->GetSynergyTags())
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

FText UCAP_SwapDetailPanelWIdget::GetGradeText(EItemGrade Grade) const
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

void UCAP_SwapDetailPanelWIdget::AddFeatureIconToBox(TSoftObjectPtr<class UTexture2D> IconPtr)
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
		HBoxSlot->SetPadding(FMargin(20.f, 0.f, 20.f, 0.f));
		HBoxSlot->SetHorizontalAlignment(HAlign_Fill);
		HBoxSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UCAP_SwapDetailPanelWIdget::AddSynergyFeatureIcon(class UCAP_SynergyDataAsset* SynergyDA)
{
	if (!SynergyDA || SynergyDA->SynergyIcon.IsNull()) return;

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
		HBoxSlot->SetPadding(FMargin(20.f, 0.f, 20.f, 0.f));
		HBoxSlot->SetHorizontalAlignment(HAlign_Fill);
		HBoxSlot->SetVerticalAlignment(VAlign_Fill);
	}
}
