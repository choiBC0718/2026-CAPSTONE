// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAP_SummonActor.generated.h"

UCLASS()
class ACAP_SummonActor : public AActor
{
	GENERATED_BODY()
	
public:
	ACAP_SummonActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY()
	class AActor* TargetPlayer;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float FollowSpeed = 5.f;
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float FollowDistance = 200.f;
};
