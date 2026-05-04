// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_SwapDetailPanelWIdget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/CAP_EquipItemEffectTypes.h"
#include "Interactables/Item/CAP_ItemInstance.h"

void UCAP_SwapDetailPanelWIdget::UpdateDetailInfo(UObject* ItemData)
{
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

	if (UCAP_ItemInstance* ItemInst = Cast<UCAP_ItemInstance>(ItemData))
	{
		if (UCAP_ItemDataBase* ItemDA = ItemInst->GetItemDA())
		{
			ItemNameText->SetText(ItemDA->ItemName);
			ItemGradeText->SetText(GetGradeText(ItemDA->ItemGrade));
			ItemDescriptionText->SetText(ItemDA->ItemDescription);

			if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn()))
			{
				if (UCAP_InventoryComponent* InventoryComp = Player->GetInventoryComponent())
				{
					const TMap<FGameplayTag, FSynergyDataTable*>& SynergyCache = InventoryComp->GetSynergyDataCache();

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
