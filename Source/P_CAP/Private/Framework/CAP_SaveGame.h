// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CAP_SaveGame.generated.h"


USTRUCT(BlueprintType)
struct FPlayerReinforceStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	int32 MaxHealthLv=0;
	UPROPERTY(VisibleAnywhere)
	int32 AttackLv=0;
};
/**
 * 
 */
UCLASS()
class UCAP_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UCAP_SaveGame();

	UPROPERTY(VisibleAnywhere)
	int32 SavedMagicStone;
	
	UPROPERTY(VisibleAnywhere)
	FPlayerReinforceStats PlayerReinforceStats;
};
