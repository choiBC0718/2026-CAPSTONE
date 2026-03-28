// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_SendComboStartEnd.generated.h"

/**
 * 
 */
UCLASS()
class UANS_SendComboStartEnd : public UAnimNotifyState
{
	GENERATED_BODY()


public:
	UANS_SendComboStartEnd();
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere, Category="Event Tag")
	FGameplayTag NextComboNameTag;
	UPROPERTY(EditAnywhere, Category="Event Tag")
	FGameplayTag ComboEndTag;

	FGameplayTag TargetClearTag;
};
