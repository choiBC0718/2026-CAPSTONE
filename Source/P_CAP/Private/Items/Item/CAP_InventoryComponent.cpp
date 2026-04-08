// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item/CAP_InventoryComponent.h"

#include "AbilitySystemBlueprintLibrary.h"


UCAP_InventoryComponent::UCAP_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UCAP_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
}

bool UCAP_InventoryComponent::AddItem(class UCAP_ItemInstance* NewItem)
{
	if (!NewItem)
		return false;

	if (InventoryItems.Num() >= Capacity)
		return false;

	InventoryItems.Add(NewItem);
	if (OnInventoryChanged.IsBound())
	{
		OnInventoryChanged.Broadcast(NewItem, true);
	}
	return true;
}
