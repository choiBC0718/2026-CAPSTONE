// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widget/Common/CAP_ItemInteraction.h"
#include "CAP_GameplayWidget.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_GameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	FORCEINLINE UCAP_ItemInteraction* GetInteractionWidget() const {return InteractionWidget;}
	
protected:
	UPROPERTY(meta = (BindWidget))
	class UCAP_ValueGauge* HealthBar;
	UPROPERTY(meta = (BindWidget))
	class UCAP_ItemInteraction* InteractionWidget;
};
