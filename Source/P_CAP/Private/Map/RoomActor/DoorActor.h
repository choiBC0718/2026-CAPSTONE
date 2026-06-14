// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DoorDirection.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DoorActor.generated.h"

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
	virtual void OnConstruction(const FTransform& Transform) override;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Door|Back Blocker")
	UBoxComponent* BackBlocker;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Back Blocker")
	bool bEnableBackBlocker = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Back Blocker")
	bool bUseDirectionBasedBackBlockerPlacement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Back Blocker", meta=(EditCondition="bEnableBackBlocker"))
	FVector BackBlockerExtent = FVector(260.f, 80.f, 180.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Back Blocker", meta=(EditCondition="bEnableBackBlocker && bUseDirectionBasedBackBlockerPlacement", ClampMin="0.0"))
	float BackBlockerOutwardDistance = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Back Blocker", meta=(EditCondition="bEnableBackBlocker && bUseDirectionBasedBackBlockerPlacement"))
	float BackBlockerZOffset = 160.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Back Blocker", meta=(EditCondition="bEnableBackBlocker", MakeEditWidget="true"))
	FVector BackBlockerRelativeLocation = FVector(-180.f, 0.f, 160.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Back Blocker")
	bool bFlipBackBlockerForVerticalDoors = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Door|Back Blocker")
	bool bRotateBackBlockerForVerticalDoors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Door|Back Blocker", meta=(EditCondition="bRotateBackBlockerForVerticalDoors"))
	float VerticalBackBlockerYawOffset = 180.f;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> PortalMaterialInstances;
	
private:
	void ApplyBackBlockerSettings();
	void ApplyDirectionBasedBackBlockerSettings();
	FVector GetAdjustedBackBlockerRelativeLocation() const;
	FRotator GetAdjustedBackBlockerRelativeRotation() const;
	FVector GetDoorOutwardWorldDirection() const;
	float GetBackBlockerWorldYaw() const;
	void InitializePortalMaterialInstances();
	void SetPortalAlphaCutoff(float NewAlphaCutoff);

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	float CurrentPortalAlphaCutoff = 0.f;
	float TargetPortalAlphaCutoff = 0.f;
};
