// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CAP_GameInstance.h"

#include "AbilitySystemGlobals.h"
#include "CAP_SaveGame.h"
#include "Kismet/GameplayStatics.h"

void UCAP_GameInstance::Init()
{
	Super::Init();
	UAbilitySystemGlobals::Get().InitGlobalData();
	LoadGameData();
}

void UCAP_GameInstance::SaveGameData()
{
	if (CurrentSaveGame)
		UGameplayStatics::SaveGameToSlot(CurrentSaveGame,SaveSlotName,UserIndex);
}

void UCAP_GameInstance::LoadGameData()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
	{
		CurrentSaveGame = Cast<UCAP_SaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));
	}
	else
	{
		CurrentSaveGame = Cast<UCAP_SaveGame>(UGameplayStatics::CreateSaveGameObject(UCAP_SaveGame::StaticClass()));
		SaveGameData();
	}
}

int32 UCAP_GameInstance::GetSavedMagicStone() const
{
	return CurrentSaveGame ? CurrentSaveGame->SavedMagicStone : 0;
}

void UCAP_GameInstance::OnCurrencyChanged(ECurrencyType Type, int32 OldAmount, int32 NewAmount)
{
	if (Type == ECurrencyType::MagicStone && CurrentSaveGame)
	{
		CurrentSaveGame->SavedMagicStone = NewAmount;
		SaveGameData();
	}
}
