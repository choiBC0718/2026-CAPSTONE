// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "CAP_SkillToolTipWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_SkillToolTipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupToolTip(const FWeaponSkillData& SkillData);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SkillNameText;
	UPROPERTY(meta = (BindWidget))
	class URichTextBlock* DescriptionText;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CooldownText;
	UPROPERTY(meta = (BindWidget))
	class UImage* SkillIcon;
};
