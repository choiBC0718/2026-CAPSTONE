// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Map/RoomActor/Interior/RoomInteriorData.h"
#include "RoomInteriorPropSet.generated.h"

USTRUCT(BlueprintType)
struct FRoomInteriorPropMeshVariant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop", meta = (ClampMin = "1"))
	int32 Weight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
	FVector2D UniformScaleRange = FVector2D(1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
	FVector LocationJitter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prop")
	float YawJitterDegrees = 0.f;
};

USTRUCT(BlueprintType)
struct FRoomInteriorPropRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
	ERoomInteriorStructureCategory Category = ERoomInteriorStructureCategory::Generic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
	FIntPoint Footprint = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rule")
	TArray<FRoomInteriorPropMeshVariant> Variants;
};

UCLASS(BlueprintType)
class URoomInteriorPropSet : public UDataAsset
{
	GENERATED_BODY()

public:
	const FRoomInteriorPropRule* FindRule(
		ERoomInteriorStructureCategory Category,
		const FIntPoint& Footprint) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Props")
	TArray<FRoomInteriorPropRule> Rules;
};
