// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Character.h"
#include "Map/RoomTypes.h"
#include "RoomMonsterSpawnDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FRoomMonsterSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster")
	TSubclassOf<ACharacter> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(ClampMin="1"))
	int32 Cost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(ClampMin="1"))
	int32 Weight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(ClampMin="1"))
	int32 MaxCount = 99;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(ClampMin="1"))
	int32 MaxMonstersPerCell = 1;
};

UENUM(BlueprintType)
enum class ERoomReinforcementTrigger : uint8
{
	WhenAliveMonsterCountBelow UMETA(DisplayName="When Alive Monster Count Below"),
	AfterCombatTime UMETA(DisplayName="After Combat Time")
};

USTRUCT(BlueprintType)
struct FRoomReinforcementRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement")
	ERoomReinforcementTrigger Trigger = ERoomReinforcementTrigger::WhenAliveMonsterCountBelow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0"))
	FIntPoint ScoreRange = FIntPoint(4, 6);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0.0"))
	float DelayAfterTriggered = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0"))
	int32 AliveMonsterThreshold = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0.0"))
	float CombatTime = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement")
	bool bRequiredForRoomClear = true;
};

USTRUCT(BlueprintType)
struct FRoomMonsterSpawnRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room")
	ERoomType RoomType = ERoomType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room", meta=(ClampMin="0"))
	FIntPoint ScoreRange = FIntPoint(10, 20);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room")
	TArray<FRoomMonsterSpawnEntry> MonsterPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Reinforcement")
	TArray<FRoomReinforcementRule> Reinforcements;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Reinforcement", meta=(ClampMin="0"))
	int32 MaxReinforcementCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Reinforcement", meta=(ClampMin="0"))
	int32 MaxReinforcementScore = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Reinforcement", meta=(ClampMin="0.0"))
	float MinReinforcementInterval = 3.f;
};

UCLASS(BlueprintType)
class URoomMonsterSpawnDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FRoomMonsterSpawnRule* FindRule(ERoomType RoomType) const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true"))
	TArray<FRoomMonsterSpawnRule> SpawnRules;
};
