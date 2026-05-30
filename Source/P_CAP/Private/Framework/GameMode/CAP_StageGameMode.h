// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_GameMode.h"
#include "CAP_StageGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_StageGameMode : public ACAP_GameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	void OnPlayerDeathAnimationFinished(class APlayerController* PC);

protected:
	UFUNCTION()
	void ShowDeathDashboard(class APlayerController* PC);
	
	UPROPERTY(EditDefaultsOnly, Category = "Death Sequence")
	float RagdollSettleTime = 2.0f;

private:
	FTimerHandle RagdollWaitTimerHandle;
};
