// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAP_TargetRangeIndicator.generated.h"

UCLASS()
class ACAP_TargetRangeIndicator : public AActor
{
	GENERATED_BODY()
	
public:	

	ACAP_TargetRangeIndicator();

	void Initialize(float NewRange);

	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* RangeDecal;
};
