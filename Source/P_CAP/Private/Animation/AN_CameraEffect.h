// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_CameraEffect.generated.h"

/**
 * 
 */
UCLASS()
class UAN_CameraEffect : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, Category="Camera FX")
	UParticleSystem* CameraParticle;
	
	UPROPERTY(EditAnywhere, Category="Camera FX")
	FTransform ParticleOffset = FTransform(FRotator::ZeroRotator, FVector(15.f, 0.f, 0.f));
	
};
