// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Map/RoomActor/Interior/RoomInteriorTemplateActor.h"
#include "Map/RoomData.h"
#include "RoomDecorationSet.generated.h"

UENUM(BlueprintType)
enum class ERoomDecorCollisionType : uint8
{
	Blocking UMETA(DisplayName="Blocking"),
	VisualOnly UMETA(DisplayName="Visual Only"),
	OverlapOnly UMETA(DisplayName="Overlap Only")
};

USTRUCT(BlueprintType)
struct FRoomTemplateDecorationRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Template")
	TSubclassOf<ARoomInteriorTemplateActor> TemplateClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Template", meta=(ClampMin="0.0"))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration", meta=(ClampMin="0"))
	int32 MinSmallDecorCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration", meta=(ClampMin="0"))
	int32 MaxSmallDecorCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration", meta=(ClampMin="0"))
	int32 MinLargeDecorCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration", meta=(ClampMin="0"))
	int32 MaxLargeDecorCount = 1;
};

USTRUCT(BlueprintType)
struct FRoomSmallDecorationEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration")
	TObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration")
	ERoomDecorCollisionType CollisionType = ERoomDecorCollisionType::VisualOnly;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration", meta=(ClampMin="0.0"))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration")
	FVector2D UniformScaleRange = FVector2D(0.9f, 1.1f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration")
	float ZOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration")
	float YawJitterDegrees = 180.f;
};

USTRUCT(BlueprintType)
struct FRoomLargeDecorationEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration")
	TSubclassOf<AActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration", meta=(ClampMin="0.0"))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration", meta=(ClampMin="1"))
	int32 MinClusterCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration", meta=(ClampMin="1"))
	int32 MaxClusterCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration", meta=(ClampMin="0"))
	int32 ClusterRadiusInCells = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration")
	FVector2D UniformScaleRange = FVector2D(1.f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration")
	float ZOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration")
	float YawJitterDegrees = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration", meta=(ClampMin="0"))
	int32 BlockRadiusInCells = 1;
};

UCLASS(BlueprintType)
class URoomDecorationSet : public UDataAsset
{
	GENERATED_BODY()

public:
	const FRoomSmallDecorationEntry* PickSmallEntry(FRandomStream& RandomStream) const;
	const FRoomLargeDecorationEntry* PickLargeEntry(FRandomStream& RandomStream) const;
	const FRoomTemplateDecorationRule* PickTemplateRule(const FRoomData& RoomData, FRandomStream& RandomStream) const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Template", meta=(AllowPrivateAccess="true"))
	TArray<FRoomTemplateDecorationRule> TemplateRules;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Small Decoration", meta=(AllowPrivateAccess="true"))
	TArray<FRoomSmallDecorationEntry> SmallDecorEntries;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Large Decoration", meta=(AllowPrivateAccess="true"))
	TArray<FRoomLargeDecorationEntry> LargeDecorEntries;
};
