// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RewardWeaponSpawner.generated.h"

class ACAP_WorldWeapon;
class UCAP_WeaponDataAsset;

UCLASS(Blueprintable)
class ARewardWeaponSpawner : public AActor
{
	GENERATED_BODY()

public:
	ARewardWeaponSpawner();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Reward")
	ACAP_WorldWeapon* SpawnRandomWeapon();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	TSubclassOf<ACAP_WorldWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	TArray<TObjectPtr<UCAP_WeaponDataAsset>> WeaponPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	FVector SpawnOffset = FVector(0.f, 0.f, 80.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	bool bSpawnOnlyOnce = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward|Special Room")
	bool bConsumeSpecialRoomRewardOnSpawn = true;

	UPROPERTY(Transient)
	TObjectPtr<ACAP_WorldWeapon> SpawnedWeapon;
};
