// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_SkillReroll.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_CurrencyComponent.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"

ENPCActionResult ANPC_SkillReroll::ExecuteSpecialAction(AActor* Actor)
{
	return ENPCActionResult::OpenCustomWidget;
}

FText ANPC_SkillReroll::GetDialogueText(EEnhanceResult Result, int32 Cost) const
{
	const TArray<FText>* TargetPool = nullptr;
	switch (Result)
	{
	case EEnhanceResult::Default: TargetPool = &DefaultDialoguePool; break;
	case EEnhanceResult::Success: TargetPool = &SuccessDialoguePool; break;
	case EEnhanceResult::InsufficientCurrency: TargetPool = &InsufficientDialoguePool; break;
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

EEnhanceResult ANPC_SkillReroll::TryRerollSkill(class ACAP_PlayerCharacter* Player)
{
	if (!Player)
		return EEnhanceResult::Error;

	UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent();
	UCAP_CurrencyComponent* CurrComp = Player->GetCurrencyComponent();
	UCAP_WeaponInstance* CurrentWeapon = WeaponComp->GetCurrentWeaponInstance();
	if (!WeaponComp || !CurrComp || !CurrentWeapon)
		return EEnhanceResult::Error;
	
	EItemGrade CurrentGrade = CurrentWeapon->GetCurrentGrade();
	int32 RequiredCost = RerollCostMap.Contains(CurrentGrade) ? RerollCostMap[CurrentGrade] : 0;
	if (CurrComp->ConsumeCurrency(ECurrencyType::Gold, RequiredCost))
	{
		WeaponComp->ClearAbilities(CurrentWeapon);
		CurrentWeapon->UnloadWeaponAssets();
		if (CurrentWeapon->RerollRandomSkill())
		{
			CurrentWeapon->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(WeaponComp, [WeaponComp, CurrentWeapon]()
			{
				if (WeaponComp && CurrentWeapon)
				{
					WeaponComp->GrantAbilities(CurrentWeapon);
					WeaponComp->OnWeaponSkillChanged.Broadcast(CurrentWeapon);
				}
			}));
			return EEnhanceResult::Success;
		}
		else
		{
			CurrentWeapon->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(WeaponComp, [WeaponComp, CurrentWeapon]()
			{
				if (WeaponComp && CurrentWeapon)
					WeaponComp->GrantAbilities(CurrentWeapon);
			}));
			return EEnhanceResult::Error;
		}
	}
	
	return EEnhanceResult::InsufficientCurrency;
}
