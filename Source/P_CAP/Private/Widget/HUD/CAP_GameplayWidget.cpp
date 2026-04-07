// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Weapon/CAP_WeaponComponent.h"
#include "Widget/Common/CAP_ValueGauge.h"
#include "Widget/Common/CAP_AbilityListView.h"

void UCAP_GameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			WeaponComp->OnWeaponChanged.AddDynamic(this, &UCAP_GameplayWidget::HandleWeaponChanged);
		}
		
		OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);
		
		if (OwnerASC && HealthBar)
		{
			HealthBar->SetAndBoundToGameplayAttribute(OwnerASC, UCAP_AttributeSet::GetHealthAttribute(), UCAP_AttributeSet::GetMaxHealthAttribute());
		}
		if (InteractionWidget)
		{
			InteractionWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
}

void UCAP_GameplayWidget::HandleWeaponChanged(class UCAP_WeaponInstance* NewWeaponInstance)
{
	if (AbilityListView && NewWeaponInstance)
	{
		AbilityListView->RefreshWeaponSkills(NewWeaponInstance);
	}
}
