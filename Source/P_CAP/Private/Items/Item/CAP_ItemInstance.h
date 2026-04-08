// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_ItemDataAsset.h"
#include "UObject/NoExportTypes.h"
#include "CAP_ItemInstance.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_ItemInstance : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(UCAP_ItemDataAsset* NewItemDA);

	UFUNCTION()
	UCAP_ItemDataAsset* GetItemDA() const {return ItemDA;}

private:
	UPROPERTY(VisibleAnywhere)
	UCAP_ItemDataAsset* ItemDA;
};
