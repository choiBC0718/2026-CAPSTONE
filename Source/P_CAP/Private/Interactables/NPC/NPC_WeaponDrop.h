// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/NPC/CAP_WorldNPC.h"
#include "NPC_WeaponDrop.generated.h"

/**
 * 
 */
UCLASS()
class ANPC_WeaponDrop : public ACAP_WorldNPC
{
	GENERATED_BODY()

public:
	virtual ENPCActionResult ExecuteSpecialAction(AActor* Actor) override;
	virtual bool GetSpecialActionCost(AActor* Actor, int32& OutCost) override;

protected:
	UPROPERTY(EditAnywhere, Category="Drop")
	TSubclassOf<class ACAP_WorldWeapon> WeaponClass;
	UPROPERTY(EditAnywhere, Category="Drop")
	TArray<class UCAP_WeaponDataAsset*> DropWeaponDataAssets;

	UPROPERTY(EditAnywhere, Category="Drop")
	int32 CostMagicStone = 10;

	UPROPERTY()
	class UCAP_WeaponDataAsset* FirstDroppedWeapon = nullptr;
	
	int32 InteractionCount = 0;
};
