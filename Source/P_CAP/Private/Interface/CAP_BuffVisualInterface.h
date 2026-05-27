// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CAP_BuffVisualInterface.generated.h"

USTRUCT(BlueprintType)
struct FBuffDisplayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
};


UINTERFACE(MinimalAPI)
class UCAP_BuffVisualInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ICAP_BuffVisualInterface
{
	GENERATED_BODY()

public:
	virtual FBuffDisplayData GetBuffDisplayData(const FGameplayTag& EffectTag) const = 0;
};
