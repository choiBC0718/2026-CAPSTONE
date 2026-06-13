// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/SpecialRoomTransitionSubsystem.h"

void USpecialRoomTransitionSubsystem::SaveReturnState(const FSpecialRoomReturnState& ReturnState)
{
	PendingReturnState = ReturnState;
	bHasPendingReturn = true;
	bBlockStageAutoStartOnce = true;
	bStageAutoStartSkippedForThisLoad = false;
}

bool USpecialRoomTransitionSubsystem::ConsumeReturnState(FSpecialRoomReturnState& OutReturnState)
{
	if (!bHasPendingReturn)
	{
		return false;
	}

	OutReturnState = PendingReturnState;
	bHasPendingReturn = false;
	bBlockStageAutoStartOnce = !bStageAutoStartSkippedForThisLoad;
	return true;
}

bool USpecialRoomTransitionSubsystem::ShouldSkipStageAutoStart() const
{
	return bHasPendingReturn || bBlockStageAutoStartOnce;
}

void USpecialRoomTransitionSubsystem::MarkStageAutoStartSkipped()
{
	bBlockStageAutoStartOnce = false;
	bStageAutoStartSkippedForThisLoad = true;
}
