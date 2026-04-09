// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "RoomPathActor.generated.h"

UCLASS()
class ARoomPathActor : public AActor
{
	GENERATED_BODY()

public:
	ARoomPathActor();

	void InitializePath(const TArray<FVector>& InWorldPathPoints);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	USplineComponent* PathSpline;
};
