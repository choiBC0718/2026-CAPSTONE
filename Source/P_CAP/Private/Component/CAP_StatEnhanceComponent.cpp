// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_StatEnhanceComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_StatEnhanceTypes.h"
#include "Framework/CAP_GameInstance.h"
#include "Kismet/GameplayStatics.h"


UCAP_StatEnhanceComponent::UCAP_StatEnhanceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCAP_StatEnhanceComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UCAP_GameInstance* GI = Cast<UCAP_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		TMap<FName, int32> SavedData = GI->GetSavedStatEnhancedLevels();
		LoadEnhanceData(SavedData);
	}
}

int32 UCAP_StatEnhanceComponent::GetStatEnhanceLevel(FName RowName)
{
	return EnhancedStatLevels.Contains(RowName) ? EnhancedStatLevels[RowName] : 0;
}

bool UCAP_StatEnhanceComponent::UpgradeStatEnhance(FName RowName, int32 MaxLevel)
{
	int32 CurrentLevel = GetStatEnhanceLevel(RowName);
	if (CurrentLevel >= MaxLevel)
		return false;
	
	EnhancedStatLevels.FindOrAdd(RowName) = CurrentLevel+1;
	ApplyStatEnhanceGE(RowName, CurrentLevel+1);

	if (UCAP_GameInstance* GI = Cast<UCAP_GameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->UpdateSavedStatEnhanceLevel(RowName, CurrentLevel+1);
		
	}
	return true;
}

void UCAP_StatEnhanceComponent::LoadEnhanceData(const TMap<FName, int32>& SavedData)
{
	EnhancedStatLevels = SavedData;
	for (const TPair<FName, int32>& StatPair : EnhancedStatLevels)
	{
		ApplyStatEnhanceGE(StatPair.Key, StatPair.Value);
	}
}

void UCAP_StatEnhanceComponent::ApplyStatEnhanceGE(FName RowName, int32 Level)
{
	if (Level <= 0 || !EnhanceDataTable)		return;
	FStatEnhanceTableRow* RowData = EnhanceDataTable->FindRow<FStatEnhanceTableRow>(RowName, "");
	if (!RowData) return;
	
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwner());
	if (!Player)		return;
	
	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player));
	if (!ASC || !ASC->GetGenerics())		return;
	
	TSubclassOf<UGameplayEffect> StatEnhanceGE = ASC->GetGenerics()->GetItemStatInfiniteEffect();
	if (!StatEnhanceGE)	return;
	const UGameplayEffect* DefaultGE = StatEnhanceGE->GetDefaultObject<UGameplayEffect>();

	if (ActiveGEHandles.Contains(RowName))
		ASC->RemoveActiveGameplayEffect(ActiveGEHandles[RowName]);

	FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(StatEnhanceGE, 1.f, EffectContext);
	if (SpecHandle.IsValid())
	{
		
#define IF_WITH_EDITOR()
		for (const FGameplayModifierInfo& ModInfo : DefaultGE->Modifiers)
		{
			if (ModInfo.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
			{
				FGameplayTag CallerTag = ModInfo.ModifierMagnitude.GetSetByCallerFloat().DataTag;
				SpecHandle.Data->SetSetByCallerMagnitude(CallerTag, 0.f);
			}
		}
#define EndIF_WITH_EDITOR()
		
		for (const FStatModifierInfo& ModInfo : RowData->Modifiers)
		{
			float CalculatedMag = ModInfo.bIsFixedValue ? ModInfo.Value : static_cast<float>(Level) * ModInfo.Value;

			SpecHandle.Data.Get()->SetSetByCallerMagnitude(ModInfo.StatTag, CalculatedMag);
		}
		
		FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		ActiveGEHandles.FindOrAdd(RowName) = ActiveHandle;
	}
}