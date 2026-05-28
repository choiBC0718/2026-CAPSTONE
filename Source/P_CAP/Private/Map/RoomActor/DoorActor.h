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
class UMaterialInstanceDynamic;

UCLASS()
class ADoorActor : public AActor
{
	GENERATED_BODY()
	
public:
	ADoorActor();

	void InitializeDoor(const FIntPoint& InSourceRoomPos, const FIntPoint& InTargetRoomPos, EDoorDirection InDirection);
	void SetPortalEnabled(bool bEnabled);

	FIntPoint GetSourceRoomPos() const { return SourceRoomPos; }
	FIntPoint GetTargetRoomPos() const { return TargetRoomPos; }
	EDoorDirection GetDoorDirection() const { return Direction; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	UStaticMeshComponent* DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door")
	UStaticMeshComponent* PortalMesh;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Portal", meta=(ClampMin="0.01"))
	float PortalTransitionDuration = 1.0f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> PortalMaterialInstances;
	
private:
	void InitializePortalMaterialInstances();
	void SetPortalAlphaCutoff(float NewAlphaCutoff);

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	float CurrentPortalAlphaCutoff = 0.f;
	float TargetPortalAlphaCutoff = 0.f;
};
