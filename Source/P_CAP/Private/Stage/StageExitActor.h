// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StageExitActor.generated.h"

class AStageManager;
class UBoxComponent;
class UPrimitiveComponent;
class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class AStageExitActor : public AActor
{
	GENERATED_BODY()

public:
	AStageExitActor();

	void SetExitEnabled(bool bNewEnabled);
	bool IsExitEnabled() const { return bEnabled; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage Exit", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage Exit", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage Exit", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> PortalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage Exit", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AStageManager> StageManager;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage Exit", meta=(AllowPrivateAccess="true"))
	bool bEnabled = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stage Exit", meta=(AllowPrivateAccess="true"))
	bool bIsProcessingExit = false;

	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
