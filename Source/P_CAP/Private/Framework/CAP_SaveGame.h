// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CAP_SaveGame.generated.h"



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
	TMap<FName, int32> SavedStatEnhanceLevels;
};
