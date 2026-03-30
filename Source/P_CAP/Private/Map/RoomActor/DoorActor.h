// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "DoorDirection.h"
#include "GameFramework/Actor.h"
#include "DoorActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

UCLASS()
class ADoorActor : public AActor
{
	GENERATED_BODY()
	
public:
	ADoorActor();

	void InitializeDoor(const FIntPoint& InSourceRoomPos, const FIntPoint& InTargetRoomPos, EDoorDirection InDirection);

	FIntPoint GetSourceRoomPos() const { return SourceRoomPos; }
	FIntPoint GetTargetRoomPos() const { return TargetRoomPos; }
	EDoorDirection GetDoorDirection() const { return Direction; }

protected:
	virtual void BeginPlay() override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	UBoxComponent* TriggerBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door")
	EDoorDirection Direction = EDoorDirection::Up;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	FIntPoint SourceRoomPos = FIntPoint::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	FIntPoint TargetRoomPos = FIntPoint::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	bool bIsProcessingMove = false;
	
private:
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
