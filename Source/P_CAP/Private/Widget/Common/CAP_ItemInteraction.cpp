// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_ItemInteraction.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Framework/CAP_GameInstance.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Kismet/GameplayStatics.h"


void UCAP_ItemInteraction::NativeConstruct()
{
	Super::NativeConstruct();
	if (InteractProgressImage)
	{
		ProgressMID = InteractProgressImage->GetDynamicMaterial();
	}
	if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
	{
		if (UCAP_InteractionComponent* InteractComp = Player->GetInteractionComponent())
			InteractComp->OnInteractProgressUpdated.AddDynamic(this, &UCAP_ItemInteraction::UpdateInteractProgress);
	}
}

void UCAP_ItemInteraction::SetInteractionUIVisibility(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	if (bVisible)
	{
		UpdateInteractProgress(0.f);
	}
}

void UCAP_ItemInteraction::UpdateInteractProgress(float Progress)
{
	if (ProgressBar)
	{
		ProgressBar->SetPercent(Progress);
	}
}

void UCAP_ItemInteraction::SetInteractKeyText(const FString& KeyName)
{
	if (UCAP_GameInstance* GI = Cast<UCAP_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UDataTable* KeyTable = GI->GetKeyIconTable())
		{
			FName RowName = FName(*KeyName);
			FKeyIconRow* Row = KeyTable->FindRow<FKeyIconRow>(RowName,"");
			if (Row && Row->Icon)
			{
				if (EquipIconImg)
				{
					EquipIconImg->SetBrushFromTexture(Row->Icon);
					EquipIconImg->SetVisibility(ESlateVisibility::Visible);
				}
				if (DisassembleIconImg)
				{
					DisassembleIconImg->SetBrushFromTexture(Row->Icon);
					DisassembleIconImg->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
	}
}

void UCAP_ItemInteraction::UpdateActionTexts(const FInteractionPayload& Payload)
{
	EquipText->SetText(FText::FromString(Payload.ActionData.ShortActionText));
	
	if (Payload.ActionData.LongActionText.IsEmpty())
	{
		HoldInteractBorder->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		HoldInteractBorder->SetVisibility(ESlateVisibility::Visible);
		
		FString LongText = Payload.ActionData.LongActionText;
		if (Payload.ActionData.bShowCurrency)
		{
			int32 FinalAmount = Payload.ActionData.CurrencyAmount;
			ECurrencyType RewardType = Payload.ActionData.ActionCurrencyType;

			if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
			{
				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player))
				{
					float BonusMul = ASC->GetNumericAttribute(UCAP_AttributeSet::GetDisassembleBonusMultiplierAttribute());
					FinalAmount = FMath::RoundToInt(FinalAmount * (1.f + BonusMul));
				}
			}
			
			LongText=FString::Printf(TEXT("%s (+%d)"), *LongText, FinalAmount);
			if (DisassembleRewardIcon)
			{
				if (UTexture2D** FoundIcon = CurrencyIconMap.Find(RewardType))
				{
					DisassembleRewardIcon->SetBrushFromTexture(*FoundIcon);
					DisassembleRewardIcon->SetVisibility(ESlateVisibility::Visible);
				}
				else
					DisassembleRewardIcon->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		else
		{
			if (DisassembleRewardIcon)
			{
				DisassembleRewardIcon->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		DisassembleText->SetText(FText::FromString(LongText));
	}
}
