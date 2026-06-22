// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/Common/CAP_SkillToolTipWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "GAS/Setting/CAP_AttributeSet.h"

void UCAP_SkillToolTipWidget::SetupToolTip(const FWeaponSkillData& SkillData)
{
	if (SkillNameText)
		SkillNameText->SetText(FText::FromName(SkillData.SkillName));

	if (CooldownText)
	{
		FString CooldownStr = TEXT("재사용 대기시간 ");
		CooldownStr += FString::Printf(TEXT("%.0f초"), SkillData.CooldownTime);
		CooldownText->SetText(FText::FromString(CooldownStr));
	}

	if (DescriptionText)
	{
		FString ColorTag = (SkillData.DamageType == ESkillDamageType::Physical) ? TEXT("PhysicalColor") : TEXT("MagicalColor");
		float ExpectedDamage = 0.f;
		if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player))
			{
				ExpectedDamage = ASC->GetNumericAttribute((SkillData.DamageType == ESkillDamageType::Physical) ? UCAP_AttributeSet::GetPhysicalDamageAttribute() : UCAP_AttributeSet::GetMagicalDamageAttribute());
				ExpectedDamage = ExpectedDamage* SkillData.DamageMultiplier + SkillData.BaseDamage;
			}
		}

		FFormatNamedArguments Args;
		Args.Add(TEXT("DamageColor"), FText::FromString(ColorTag));
		Args.Add(TEXT("TotalDamage"), FText::AsNumber(FMath::RoundToInt(ExpectedDamage)));
		DescriptionText->SetText(FText::Format(SkillData.Description, Args));
	}

	if (SkillIcon)
		SkillIcon->SetBrushFromTexture(SkillData.SkillIcon.LoadSynchronous());
}
