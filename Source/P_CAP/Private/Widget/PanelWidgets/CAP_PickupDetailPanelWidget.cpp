// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_PickupDetailPanelWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InteractionComponent.h"
#include "Components/TextBlock.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"
#include "Interface/CAP_InteractInterface.h"
#include "Widget/Common/CAP_ItemInteraction.h"

void UCAP_PickupDetailPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>();
	
	if (Player)
	{
		if (UCAP_InteractionComponent* InteractionComp = Player->GetInteractionComponent())
		{
			InteractionComp->OnInteractProgressUpdated.AddDynamic(this, &UCAP_PickupDetailPanelWidget::HandleUpdateInteractProgress);
			InteractionComp->OnInteractableChanged.AddDynamic(this, &UCAP_PickupDetailPanelWidget::HandleInteractableChanged);
		}
	}
}

void UCAP_PickupDetailPanelWidget::UpdateDetailInfo(UObject* ItemData)
{
	Super::UpdateDetailInfo(ItemData);

	if (UCAP_WeaponInstance* WeaponInst = Cast<UCAP_WeaponInstance>(ItemData))
	{
		if (UCAP_WeaponDataAsset* WeaponDA = WeaponInst->GetWeaponDA())
		{
			ItemNameText->SetText(WeaponDA->ItemName);
			ItemGradeText->SetText(GetGradeText(WeaponDA->ItemGrade));
			ItemDescriptionText->SetText(WeaponDA->ItemDescription);

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

void UCAP_PickupDetailPanelWidget::HandleUpdateInteractProgress(float Progress)
{
	if (InteractTextWidget)
		InteractTextWidget->UpdateInteractProgress(Progress);
}

void UCAP_PickupDetailPanelWidget::HandleInteractableChanged(AActor* InteractableActor)
{
	if (!Player)
		return;
	
	if (InteractableActor)
	{
		if (ICAP_InteractInterface* Interface = Cast<ICAP_InteractInterface>(InteractableActor))
		{
			FInteractionPayload Payload= Interface->GetInteractionPayload();
			FString KeyName = Player->GetInteractKeyName();
			UpdateInteractionUI(true,Payload.DetailData,KeyName);
		}
	}
	else
	{
		UpdateInteractionUI(false,nullptr,"");
	}
}

