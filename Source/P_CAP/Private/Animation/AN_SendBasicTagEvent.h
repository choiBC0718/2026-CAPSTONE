// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_SendBasicTagEvent.generated.h"

/**
 * 
 */
UCLASS()
class UAN_SendBasicTagEvent : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	
	UPROPERTY(EditAnywhere, Category="Event", meta=(Categories="Ability.Event"))
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category="Projectile")
	FName ProjectileMuzzleName = NAME_None;
};
