// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Subsystem/CAP_SynergySubsystem.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Data/CAP_SynergyDataAsset.h"

void UCAP_SynergySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	
	AssetRegistry.SearchAllAssets(true);
	
	TArray<FAssetData> AssetDataList;
	AssetRegistry.GetAssetsByClass(UCAP_SynergyDataAsset::StaticClass()->GetClassPathName(), AssetDataList);
	
	for (const FAssetData& AssetData : AssetDataList)
	{
		FString TagString;
		if (AssetData.GetTagValue(FName("SynergyTag"), TagString))
		{
			FString CleanTagStr = TagString;
			if (CleanTagStr.Contains(TEXT("TagName=")))
			{
				CleanTagStr.Split(TEXT("\""), nullptr, &CleanTagStr);
				CleanTagStr.Split(TEXT("\""), &CleanTagStr, nullptr);
			}
			
			FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*CleanTagStr));
			
			if (Tag.IsValid())
			{
				SynergyMap.Add(Tag, TSoftObjectPtr<UCAP_SynergyDataAsset>(AssetData.ToSoftObjectPath()));
			}
		}
	}
}
