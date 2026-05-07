// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/CAP_InteractableBase.h"
#include "CAP_WorldNPC.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_WorldNPC : public ACAP_InteractableBase
{
	GENERATED_BODY()

public:
	ACAP_WorldNPC();

	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USkeletalMeshComponent* NPCMesh;
};
