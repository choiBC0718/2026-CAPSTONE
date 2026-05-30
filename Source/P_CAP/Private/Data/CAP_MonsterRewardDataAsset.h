// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CAP_MonsterRewardDataAsset.generated.h"

class ACAP_EnemyCharacter;

UENUM(BlueprintType)
enum class ECAPMonsterRewardGroup : uint8
{
	None,
	Normal,
	Elite,
	MiniBoss,
	Boss,
};

USTRUCT(BlueprintType)
struct FCAPMonsterCurrencyReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Currency", meta=(ClampMin="0"))
	FIntPoint GoldRewardAmountRange = FIntPoint(5, 5);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Currency")
	bool bCanDropMagicStone = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Currency", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MagicStoneDropChance = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Currency", meta=(ClampMin="0"))
	FIntPoint MagicStoneRewardAmountRange = FIntPoint(1, 1);
};

USTRUCT(BlueprintType)
struct FCAPMonsterRewardGroupEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	ECAPMonsterRewardGroup RewardGroup = ECAPMonsterRewardGroup::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	FCAPMonsterCurrencyReward Reward;
};

USTRUCT(BlueprintType)
struct FCAPMonsterRewardOverrideEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	TSubclassOf<ACAP_EnemyCharacter> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	FCAPMonsterCurrencyReward Reward;
};

UCLASS(BlueprintType)
class UCAP_MonsterRewardDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	bool FindRewardForMonster(
		TSubclassOf<ACAP_EnemyCharacter> MonsterClass,
		ECAPMonsterRewardGroup RewardGroup,
		FCAPMonsterCurrencyReward& OutReward) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reward")
	FCAPMonsterCurrencyReward DefaultReward;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reward", meta=(TitleProperty="RewardGroup"))
	TArray<FCAPMonsterRewardGroupEntry> GroupRewards;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Reward", meta=(TitleProperty="MonsterClass"))
	TArray<FCAPMonsterRewardOverrideEntry> MonsterOverrides;
};
