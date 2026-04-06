// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_GameplayWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Widget/Common/CAP_ValueGauge.h"
#include "Widget/Common/CAP_AbilityListView.h"

void UCAP_GameplayWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	
	if (OwnerASC && HealthBar)
	{
		HealthBar->SetAndBoundToGameplayAttribute(OwnerASC, UCAP_AttributeSet::GetHealthAttribute(), UCAP_AttributeSet::GetMaxHealthAttribute());
	}
	if (InteractionWidget)
	{
		InteractionWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UCAP_GameplayWidget::ConfigureAbilities(const TMap<EAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
	AbilityListView->ConfigureAbilities(Abilities);
}
