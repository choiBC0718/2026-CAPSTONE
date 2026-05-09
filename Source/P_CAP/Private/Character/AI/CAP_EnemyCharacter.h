// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CAP_Character.h"
#include "CAP_EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_EnemyCharacter : public ACAP_Character
{
	GENERATED_BODY()

public:
	ACAP_EnemyCharacter();
	virtual void BeginPlay() override;
};
