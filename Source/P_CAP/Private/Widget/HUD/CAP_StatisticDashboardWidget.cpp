// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/HUD/CAP_StatisticDashboardWidget.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/WrapBox.h"
#include "Framework/CAP_GameInstance.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/Common/CAP_ItemInteraction.h"
#include "Widget/SlotWidgets/CAP_CurrencySlotWidget.h"
#include "Widget/SlotWidgets/CAP_ItemSlotWidget.h"
#include "Widget/SlotWidgets/CAP_StatisicSlotWidget.h"

void UCAP_StatisticDashboardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ReturnBtn)
		ReturnBtn->OnClicked.AddDynamic(this, &UCAP_StatisticDashboardWidget::OnReturnBtnClicked);
}

void UCAP_StatisticDashboardWidget::NativeOpenMenu()
{
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI)
		return;
	UCAP_ProgressionSubsystem* Subsys = GI->GetSubsystem<UCAP_ProgressionSubsystem>();
	if (!Subsys)
		return;

	const FRunStatistics& Stats = Subsys->GetRunStatistics();
	if (PlayTimeSlot)
		PlayTimeSlot->SetStatisticData(FormatPlayTime(Stats.TotalPlayTime));
	if (DefeatedEnemySlot)
		DefeatedEnemySlot->SetStatisticData(FormatNumber(Stats.EnemiesDefeated));
	if (TotalDamageSlot)
		TotalDamageSlot->SetStatisticData(FormatNumber(Stats.TotalDamageDeal));
	if (MaxDamageSlot)
		MaxDamageSlot->SetStatisticData(FormatNumber(Stats.MaxDamageDeal));
	if (TakenDamageSlot)
		TakenDamageSlot->SetStatisticData(FormatNumber(Stats.TotalDamageTaken));
	if (HealSlot)
		HealSlot->SetStatisticData(FormatNumber(Stats.TotalHealing));

	if (GoldSlot)
		GoldSlot->SetCurrencyText(Stats.TotalGetGold);
	if (WeaponMatSlot)
		WeaponMatSlot->SetCurrencyText(Stats.TotalGetWeaponMaterial);
	if (MagicStoneSlot)
		MagicStoneSlot->SetCurrencyText(Stats.TotalGetMagicStone);

	if (OpenAnim)
		PlayAnimation(OpenAnim);

	BringEquipments();
	SetInteractionKey();
}

void UCAP_StatisticDashboardWidget::NativeCloseMenu()
{
}

void UCAP_StatisticDashboardWidget::HandleUIConfirmInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (TriggerEvent == ETriggerEvent::Started)
		OnReturnBtnClicked();
}

void UCAP_StatisticDashboardWidget::OnReturnBtnClicked()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UCAP_ProgressionSubsystem* Subsys = GI->GetSubsystem<UCAP_ProgressionSubsystem>())
			Subsys->ClearRunStats();
	}
	OnMenuClosed.Broadcast();
	UGameplayStatics::SetGamePaused(GetWorld(), false);
	UGameplayStatics::OpenLevel(this, VillageLevelName);
}

void UCAP_StatisticDashboardWidget::BringEquipments()
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn());
	if (!Player)
		return;

	if (EquipmentWrapBox && ItemSlotClass)
	{
		EquipmentWrapBox->ClearChildren();

		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			const TArray<UCAP_WeaponInstance*> EquippedWeapons = WeaponComp->GetEquippedWeapons();
			for (int32 i = 0; i < EquippedWeapons.Num(); i++)
			{
				if (UCAP_WeaponInstance* WeaponInst = EquippedWeapons[i])
				{
					UTexture2D* Icon = (WeaponInst && WeaponInst->GetWeaponDA()) ? WeaponInst->GetWeaponDA()->ItemIcon.LoadSynchronous() : nullptr;
					UCAP_ItemSlotWidget* NewSlot = CreateWidget<UCAP_ItemSlotWidget>(this, ItemSlotClass);
					if (NewSlot && Icon)
					{
						NewSlot->InitSlot(ESlotItemType::Weapon,Icon,nullptr);
						EquipmentWrapBox->AddChild(NewSlot);
					}
				}
			}
		}

		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
		{
			const TArray<UCAP_ItemInstance*>& EquipItems = InvComp->GetInventoryItems();
			for (int32 i = 0; i < EquipItems.Num(); i++)
			{
				if (UCAP_ItemInstance* ItemInst = EquipItems[i])
				{
					UTexture2D* Icon = (ItemInst && ItemInst->GetItemDA()) ? ItemInst->GetItemDA()->ItemIcon.LoadSynchronous() : nullptr;
					UCAP_ItemSlotWidget* NewSlot = CreateWidget<UCAP_ItemSlotWidget>(this, ItemSlotClass);
					if (NewSlot && Icon)
					{
						NewSlot->InitSlot(ESlotItemType::Item,Icon,nullptr);
						EquipmentWrapBox->AddChild(NewSlot);
					}
				}
			}
		}
	}
}

void UCAP_StatisticDashboardWidget::SetInteractionKey()
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwningPlayerPawn());
	if (!Player) return;
	
	FName RowName = FName(*Player->GetInteractKeyName());
	if (UCAP_GameInstance* GI = Cast<UCAP_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		if (UDataTable* KeyTable = GI->GetKeyIconTable())
		{
			FKeyIconRow* Row = KeyTable->FindRow<FKeyIconRow>(RowName, ""); 
			if (Row && ReturnKeyIcon)
				ReturnKeyIcon->SetBrushFromTexture(Row->Icon);
		}
	}
}

FText UCAP_StatisticDashboardWidget::FormatPlayTime(float TotalSeconds)
{
	int32 Hours = TotalSeconds / 3600.f;
	int32 Min = FMath::FloorToInt(TotalSeconds/60.f);
	int32 Sec = FMath::FloorToInt(FMath::Fmod(TotalSeconds, 60.f));
	return FText::FromString(FString::Printf(TEXT("%02d : %02d : %02d"), Hours,Min, Sec));
}

FText UCAP_StatisticDashboardWidget::FormatNumber(float Number)
{
	return FText::AsNumber(FMath::RoundToInt(Number));
}
