// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/CAP_MonsterRewardDataAsset.h"

#include "Character/AI/CAP_EnemyCharacter.h"

bool UCAP_MonsterRewardDataAsset::FindRewardForMonster(
	TSubclassOf<ACAP_EnemyCharacter> MonsterClass,
	ECAPMonsterRewardGroup RewardGroup,
	FCAPMonsterCurrencyReward& OutReward) const
{
	for (const FCAPMonsterRewardOverrideEntry& Entry : MonsterOverrides)
	{
		if (Entry.MonsterClass && MonsterClass == Entry.MonsterClass)
		{
			OutReward = Entry.Reward;
			return true;
		}
	}

	for (const FCAPMonsterRewardOverrideEntry& Entry : MonsterOverrides)
	{
		if (Entry.MonsterClass && MonsterClass && MonsterClass->IsChildOf(Entry.MonsterClass))
		{
			OutReward = Entry.Reward;
			return true;
		}
	}

	for (const FCAPMonsterRewardGroupEntry& Entry : GroupRewards)
	{
		if (Entry.RewardGroup == RewardGroup)
		{
			OutReward = Entry.Reward;
			return true;
		}
	}

	OutReward = DefaultReward;
	return true;
}
