// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_StatEnhance.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_StatEnhanceComponent.h"
#include "Data/CAP_StatEnhanceTypes.h"
#include "Widget/SlotWidgets/CAP_StatEnhanceSlotWidget.h"

ENPCActionResult ANPC_StatEnhance::ExecuteSpecialAction(AActor* Actor)
{
	return ENPCActionResult::OpenCustomWidget;
}

bool ANPC_StatEnhance::TryGetEnhanceData(class UCAP_StatEnhanceSlotWidget* SlotWidget,
	class ACAP_PlayerCharacter* Player, struct FStatEnhanceTableRow*& OutRowData, FName& OutRowName,
	int32& OutCurrentLevel)
{
	if (!SlotWidget || SlotWidget->StatEnhanceDataTableRow.IsNull() || !Player)
		return false;
	if (!Player->GetStatEnhanceComponent())
		return false;

	OutRowData = SlotWidget->StatEnhanceDataTableRow.GetRow<FStatEnhanceTableRow>("");
	if (!OutRowData)
		return false;

	OutRowName = SlotWidget->StatEnhanceDataTableRow.RowName;
	OutCurrentLevel = Player->GetStatEnhanceComponent()->GetStatEnhanceLevel(OutRowName);
	return true;
}


FText ANPC_StatEnhance::GetDialogueText(EEnhanceResult Result, int32 Cost)
{
	const TArray<FText>* TargetPool = nullptr;
	
	switch (Result)
	{
	case EEnhanceResult::Default: TargetPool = &DefaultDialoguePool; break;
	case EEnhanceResult::Success: TargetPool = &SuccessDialoguePool; break;
	case EEnhanceResult::InsufficientCurrency: TargetPool = &InsufficientDialoguePool; break;
	case EEnhanceResult::MaxGradeReached: TargetPool = &OnMaxLevelDialoguePool; break;
	case EEnhanceResult::ConfirmMode: TargetPool = &ConfirmDialoguePool; break;
	default: return FText::FromString("에러 발생");
	}

	if (TargetPool && TargetPool->Num() > 0)
	{
		int32 RandIdx = FMath::RandRange(0, TargetPool->Num() - 1);
		if (Cost >= 0) // Cost 포맷팅이 필요하면 처리
		{
			FFormatNamedArguments Args;
			Args.Add(TEXT("Cost"), Cost);
			return FText::Format((*TargetPool)[RandIdx], Args);
		}
		return (*TargetPool)[RandIdx];
	}
	return FText::GetEmpty();
}

FText ANPC_StatEnhance::GetFormattedDescription(const struct FStatEnhanceTableRow* RowData, int32 CurrentLevel)
{
	if (!RowData)
		return FText::GetEmpty();

	FFormatNamedArguments Args;
	for (int32 i = 0; i < RowData->Modifiers.Num(); ++i)
	{
		const FStatModifierInfo& ModInfo = RowData->Modifiers[i];
		float CalcVal = CurrentLevel == 0 ? 0.f : ModInfo.bIsFixedValue ? ModInfo.Value : static_cast<float>(CurrentLevel) * ModInfo.Value;
		
		if (ModInfo.bIsPercentage)
			CalcVal *= 100.f;

		FString ArgName = FString::Printf(TEXT("Value%d"), i);
		Args.Add(ArgName, FMath::RoundToInt(CalcVal));
	}
	return FText::Format(RowData->Description, Args);
}

EEnhanceResult ANPC_StatEnhance::TryEnhanceStat(class ACAP_PlayerCharacter* Player, FName StatName, int32 RequiredCost, int32 MaxLevel)
{
	if (!Player)
		return EEnhanceResult::Error;

	UCAP_StatEnhanceComponent* StatComp = Player->GetStatEnhanceComponent();
	UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent();
	if (!StatComp || !CurrComp)
		return EEnhanceResult::Error;

	if (StatComp->GetStatEnhanceLevel(StatName) >= MaxLevel)
	{
		return EEnhanceResult::MaxGradeReached;
	}
	if (CurrComp->ConsumeCurrency(ECurrencyType::MagicStone, RequiredCost))
	{
		StatComp->UpgradeStatEnhance(StatName, MaxLevel);
		return EEnhanceResult::Success;
	}
	
	return EEnhanceResult::InsufficientCurrency;
}
