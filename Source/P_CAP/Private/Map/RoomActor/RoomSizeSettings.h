// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoomSizeSettings.generated.h"

UCLASS(BlueprintType)
class URoomSizeSettings : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size", meta=(ClampMin="1.0"))
	float RoomHalfExtent = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size", meta=(ClampMin="0.0"))
	float RoomGap = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size|Door", meta=(ClampMin="0.0"))
	float DoorInset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size|Door", meta=(ClampMin="0.0"))
	float EntranceInset = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size|Trigger", meta=(ClampMin="0.0"))
	float TriggerInset = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size|Interior", meta=(ClampMin="1.0"))
	float InteriorCellSize = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size|Interior", meta=(ClampMin="0.0"))
	float InteriorMargin = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size|Floor")
	bool bAutoScaleFloorMesh = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Size|Floor", meta=(EditCondition="bAutoScaleFloorMesh", ClampMin="1.0"))
	float BaseFloorSize = 2000.f;

	UFUNCTION(BlueprintPure, Category="Room Size")
	float GetRoomSize() const { return RoomHalfExtent * 2.f; }

	UFUNCTION(BlueprintPure, Category="Room Size")
	float GetRoomSpacing() const { return GetRoomSize() + RoomGap; }

	UFUNCTION(BlueprintPure, Category="Room Size|Trigger")
	float GetTriggerHalfExtent() const { return FMath::Max(100.f, RoomHalfExtent - TriggerInset); }

	UFUNCTION(BlueprintPure, Category="Room Size|Floor")
	float GetFloorScale() const { return GetRoomSize() / FMath::Max(BaseFloorSize, 1.f); }
};
