// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/CAP_InteractableBase.h"
#include "CAP_ChestGoldReward.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_ChestGoldReward : public ACAP_InteractableBase
{
	GENERATED_BODY()

public:
	ACAP_ChestGoldReward();
	virtual void BeginPlay() override;

	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class UStaticMeshComponent* GoldMesh;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class URotatingMovementComponent* RotatingMovement;

	UPROPERTY(EditDefaultsOnly, Category="Reward")
	int32 GoldAmount = 200;
};
