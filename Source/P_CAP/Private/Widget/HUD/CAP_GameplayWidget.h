// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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


protected:
	UPROPERTY(meta = (BindWidget))
	class UCAP_ValueGauge* HealthBar;
};
