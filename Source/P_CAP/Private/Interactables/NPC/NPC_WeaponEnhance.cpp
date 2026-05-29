// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_WeaponEnhance.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_WeaponComponent.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"

ENPCActionResult ANPC_WeaponEnhance::ExecuteSpecialAction(AActor* Actor)
{
	return ENPCActionResult::OpenCustomWidget;
}

EEnhanceResult ANPC_WeaponEnhance::TryUpgradeWeapon(class ACAP_PlayerCharacter* Player)
{
	if (!Player)
		return EEnhanceResult::Error;

	UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent();
	UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent();
	if (!WeaponComp || !CurrComp)
		return EEnhanceResult::Error;

	UCAP_WeaponInstance* CurrentWeapon = WeaponComp->GetCurrentWeaponInstance();
	if (!CurrentWeapon)
		return EEnhanceResult::Error;

	EItemGrade CurrentGrade = CurrentWeapon->GetCurrentGrade();
	if (CurrentGrade>=EItemGrade::Legendary)
		return EEnhanceResult::MaxGradeReached;

	int32 RequiredCost = UpgradeCostMap.Contains(CurrentGrade) ? UpgradeCostMap[CurrentGrade] : 0;
	if (CurrComp->ConsumeCurrency(ECurrencyType::WeaponMaterial, RequiredCost))
	{
		if (CurrentWeapon->UpgradeWeapon())
		{
			CurrentWeapon->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(WeaponComp, [WeaponComp, CurrentWeapon]()
			{
				if (WeaponComp && CurrentWeapon)
					WeaponComp->OnWeaponSkillChanged.Broadcast(CurrentWeapon);
			}));
			WeaponComp->ClearAbilities(CurrentWeapon);
			WeaponComp->GrantAbilities(CurrentWeapon);
			return EEnhanceResult::Success;
		}
	}
	return EEnhanceResult::InsufficientCurrency;
}

FText ANPC_WeaponEnhance::GetDialogueText(EEnhanceResult Result, int32 Cost) const
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
