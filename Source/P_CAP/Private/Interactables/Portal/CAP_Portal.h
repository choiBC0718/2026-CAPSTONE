// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/CAP_InteractableBase.h"
#include "TimerManager.h"
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
	
	virtual void BeginPlay() override;
	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class UStaticMeshComponent* PortalMesh;
	
	UPROPERTY(EditAnywhere, Category="Interact")
	FName NextStageName;

	UPROPERTY(EditAnywhere, Category="Interact")
	FText InteractionText = FText::FromString(TEXT("Exit"));

	UPROPERTY(EditAnywhere, Category="Interact|Visual")
	bool bShowPortalMesh = true;

	UPROPERTY(EditAnywhere, Category="Interact|Loading")
	TSubclassOf<class UStageLoadingWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, Category="Interact|Loading", meta=(ClampMin="0.0"))
	float OpenLevelDelay = 0.15f;

	UPROPERTY(Transient)
	TObjectPtr<class UStageLoadingWidget> ActiveLoadingWidget;

	bool bIsOpeningLevel = false;
	FTimerHandle LoadingProgressTimerHandle;
	float LoadingElapsedTime = 0.f;

	void UpdateLoadingProgress();
	void OpenTargetLevel();
};
