// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/CAP_ItemDataAsset.h"

void UCAP_ItemDataAsset::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateCachedData();
}

void UCAP_ItemDataAsset::PostLoad()
{
	Super::PostLoad();
	UpdateCachedData();
}

void UCAP_ItemDataAsset::UpdateCachedData()
{

}
