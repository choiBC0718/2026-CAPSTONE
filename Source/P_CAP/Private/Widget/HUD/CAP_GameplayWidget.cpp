// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "CAP_CharacterMenuWidget.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Items/Weapon/CAP_WeaponComponent.h"
#include "Widget/Common/CAP_ValueGauge.h"
#include "Widget/Common/CAP_AbilityListView.h"

void UCAP_GameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ACAP_PlayerCharacter* Player = GetOwningPlayerPawn<ACAP_PlayerCharacter>())
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			// 무기 변경 델리게이트 연결
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

bool UCAP_GameplayWidget::IsCharacterMenuOpen()
{
	return CharacterMenuWidget && CharacterMenuWidget->IsVisible();
}

void UCAP_GameplayWidget::OpenCharacterMenu()
{
	if (CharacterMenuWidget)
	{
		CharacterMenuWidget->SetVisibility(ESlateVisibility::Visible);
		CharacterMenuWidget->RefreshMenu();
	}
}

void UCAP_GameplayWidget::CloseCharacterMenu()
{
	if (CharacterMenuWidget)
		CharacterMenuWidget->SetVisibility(ESlateVisibility::Hidden);
}

void UCAP_GameplayWidget::SwitchCharacterMenuTab()
{
	if (CharacterMenuWidget)
		CharacterMenuWidget->SwitchNextTab();
}

void UCAP_GameplayWidget::HandleWeaponChanged(class UCAP_WeaponInstance* NewWeaponInstance)
{
	if (AbilityListView && NewWeaponInstance)
	{
		AbilityListView->RefreshWeaponSkills(NewWeaponInstance);
	}
}
