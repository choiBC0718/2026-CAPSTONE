// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CAP_TargetUIInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCAP_TargetUIInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ICAP_TargetUIInterface
{
	GENERATED_BODY()

public:
	virtual void UpdateStackUI(const FGameplayTag& BehaviorTag, int32 CurrentStack, int32 MaxStack) =0;
};
