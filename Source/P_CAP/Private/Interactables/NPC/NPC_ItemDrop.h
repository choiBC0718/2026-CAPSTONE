// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/NPC/CAP_WorldNPC.h"
#include "NPC_ItemDrop.generated.h"

/**
 * 
 */
UCLASS()
class ANPC_ItemDrop : public ACAP_WorldNPC
{
	GENERATED_BODY()

public:
	virtual ENPCActionResult ExecuteSpecialAction(AActor* Actor) override;

protected:
	UPROPERTY(EditAnywhere, Category="Drop")
	TSubclassOf<class ACAP_WorldItem> ItemClass;
	UPROPERTY(EditAnywhere, Category="Drop")
	TArray<class UCAP_ItemDataAsset*> DripItemDataAssets;
};
