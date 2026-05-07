// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Widget/PanelWidgets/CAP_SwapDetailPanelWIdget.h"
#include "CAP_PickupDetailPanelWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_PickupDetailPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void UpdateInteractionUI(bool bVisible, const FInteractionPayload& Payload, const FString& KeyName);
protected:
	UPROPERTY(meta=(BindWidget))
	class UCAP_SwapDetailPanelWIdget* ItemDetailPanelWidget;
	UPROPERTY(meta=(BindWidget))
	class UCAP_ItemInteraction* InteractTextWidget;

private:
	UPROPERTY()
	ACAP_PlayerCharacter* Player;
	
	UFUNCTION()
	void HandleUpdateInteractProgress(float Progress);
	UFUNCTION()
	void HandleInteractableChanged(AActor* InteractableActor);
};
