// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/PanelWidgets/CAP_PickupDetailPanelWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InteractionComponent.h"
#include "Components/TextBlock.h"
#include "Interface/CAP_InteractInterface.h"
#include "Widget/Common/CAP_ItemInteraction.h"

void UCAP_PickupDetailPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
	{
		if (UCAP_InteractionComponent* InteractionComp = Player->GetInteractionComponent())
		{
			InteractionComp->OnInteractableChanged.AddDynamic(this, &UCAP_PickupDetailPanelWidget::HandleInteractableChanged);
			InteractionComp->OnDialogueTriggered.AddDynamic(this, &UCAP_PickupDetailPanelWidget::OnNPCDialogueStarted);
		}
	}
}

void UCAP_PickupDetailPanelWidget::UpdateInteractionUI(bool bVisible, const FInteractionPayload& Payload, const FString& KeyName)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (!bVisible)
		return;

	if (InteractTextWidget)
	{
		InteractTextWidget->SetInteractKeyText(KeyName);
		InteractTextWidget->UpdateActionTexts(Payload);		
	}
	if (ItemDetailPanelWidget)
	{
		if (Payload.DetailData != nullptr)
		{
			ItemDetailPanelWidget->SetVisibility(ESlateVisibility::Visible);
			ItemDetailPanelWidget->UpdateDetailInfo(Payload.DetailData);
		}
		else
		{
			ItemDetailPanelWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}


void UCAP_PickupDetailPanelWidget::HandleInteractableChanged(AActor* InteractableActor)
{
	if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
	{
		if (InteractableActor)
		{
			if (ICAP_InteractInterface* Interface = Cast<ICAP_InteractInterface>(InteractableActor))
			{
				FInteractionPayload Payload= Interface->GetInteractionPayload();
				FString KeyName = Player->GetInteractKeyName();
				UpdateInteractionUI(true,Payload,KeyName);
			}
		}
		else
		{
			UpdateInteractionUI(false);
		}
	}
}

void UCAP_PickupDetailPanelWidget::OnNPCDialogueStarted(const FNPCData& NPCData)
{
	SetVisibility(ESlateVisibility::Collapsed);
}

