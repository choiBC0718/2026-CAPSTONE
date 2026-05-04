// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Item/CAP_ItemInstance.h"

void UCAP_ItemInstance::Initialize(UCAP_ItemDataBase* NewItemDA)
{
	ItemDA = NewItemDA;
}

void UCAP_ItemInstance::SetCachedASC(UCAP_AbilitySystemComponent* ASC)
{
	CachedOwnerASC = ASC;
}

UCAP_AbilitySystemComponent* UCAP_ItemInstance::GetCachedASC() const
{
	return CachedOwnerASC.IsValid() ? CachedOwnerASC.Get() : nullptr;
}
