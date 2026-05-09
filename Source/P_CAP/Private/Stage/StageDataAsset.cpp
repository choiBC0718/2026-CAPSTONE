// Fill out your copyright notice in the Description page of Project Settings.

#include "Stage/StageDataAsset.h"

const FStageConfig* UStageDataAsset::GetStageConfig(int32 StageIndex) const
{
	return Stages.IsValidIndex(StageIndex) ? &Stages[StageIndex] : nullptr;
}
