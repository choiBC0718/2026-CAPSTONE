// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/NPC/CAP_WorldNPC.h"
#include "NPC_StatEnhance.generated.h"

/**
 * 
 */
UCLASS()
class ANPC_StatEnhance : public ACAP_WorldNPC
{
	GENERATED_BODY()

public:
	virtual ENPCActionResult ExecuteSpecialAction(AActor* Actor) override;
};
