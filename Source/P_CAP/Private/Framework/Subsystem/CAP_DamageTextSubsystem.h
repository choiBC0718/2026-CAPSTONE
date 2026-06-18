// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CAP_DamageTextSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_DamageTextSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	void ShowDamage(AActor* TargetActor, float Damage, bool bIsCritical, bool bIsPlayer);

private:
	UPROPERTY()
	TArray<class ACAP_DamageTextActor*> TextActorPool;

	class ACAP_DamageTextActor* GetActorFromPool();
};
