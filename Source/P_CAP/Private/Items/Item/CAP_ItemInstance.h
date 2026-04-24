// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_ItemDataAsset.h"
#include "CAP_ItemInstance.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemInstance : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UCAP_ItemDataBase* NewItemDA);

	UFUNCTION()
	UCAP_ItemDataBase* GetItemDA() const {return ItemDA;}

protected:
	UPROPERTY(VisibleAnywhere)
	UCAP_ItemDataBase* ItemDA;
};
