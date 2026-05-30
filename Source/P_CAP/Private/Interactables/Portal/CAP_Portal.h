// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/CAP_InteractableBase.h"
#include "CAP_Portal.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_Portal : public ACAP_InteractableBase
{
	GENERATED_BODY()

public:
	ACAP_Portal();
	
	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class UStaticMeshComponent* PortalMesh;
	
	UPROPERTY(EditDefaultsOnly, Category="Interact")
	FName NextStageName;
};
