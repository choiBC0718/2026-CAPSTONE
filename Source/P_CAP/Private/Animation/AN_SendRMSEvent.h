// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_SendRMSEvent.generated.h"

/**
 * 
 */
UCLASS()
class UAN_SendRMSEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	
	UPROPERTY(EditAnywhere, Category="Event", meta=(Categories="Ability.Event"))
	FGameplayTag EventTag;
	UPROPERTY(EditAnywhere, Category="Value")
	float RMSStrength = 100.f;
	UPROPERTY(EditAnywhere, Category="Value")
	float RMSDuration = 0.1f;
};
