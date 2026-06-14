// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "RoomInteriorTemplateActor.generated.h"

USTRUCT(BlueprintType)
struct FRoomInteriorFloorExclusionBox
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Floor Exclusion")
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Floor Exclusion", meta=(ClampMin="0.0"))
	FVector Extent = FVector(500.f, 500.f, 200.f);
};

UCLASS(Blueprintable, BlueprintType)
class ARoomInteriorTemplateActor : public AActor
{
	GENERATED_BODY()

public:
	ARoomInteriorTemplateActor();

	UFUNCTION(BlueprintCallable, Category="Room Interior Template")
	void InitializeTemplate(int32 InMapSeed, const FIntPoint& InRoomGridPos);

	UFUNCTION(BlueprintImplementableEvent, Category="Room Interior Template")
	void OnTemplateInitialized(int32 InMapSeed, FIntPoint InRoomGridPos);

	const TArray<FRoomInteriorFloorExclusionBox>& GetFloorExclusionBoxes() const { return FloorExclusionBoxes; }
	void GetAllFloorExclusionBoxes(TArray<FRoomInteriorFloorExclusionBox>& OutBoxes) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room Interior Template")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room Interior Template")
	int32 MapSeed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Room Interior Template")
	FIntPoint RoomGridPos = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Interior Template|Floor Exclusion")
	TArray<FRoomInteriorFloorExclusionBox> FloorExclusionBoxes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Interior Template|Floor Exclusion")
	TArray<TObjectPtr<UBoxComponent>> FloorExclusionBoxComponents;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Interior Template|Floor Exclusion")
	FName FloorExclusionComponentTag = TEXT("FloorExclusion");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room Interior Template|Floor Exclusion")
	FString FloorExclusionComponentNamePrefix = TEXT("FloorExclusion");
};
