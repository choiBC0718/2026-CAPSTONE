// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_CameraEffect.generated.h"

/**
 * 
 */
UCLASS()
class UANS_CameraEffect : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, Category="Camera FX")
	UParticleSystem* CameraParticle;
	
	UPROPERTY(EditAnywhere, Category="Camera FX")
	FTransform ParticleOffset = FTransform(FRotator::ZeroRotator, FVector(15.f, 0.f, 0.f));
	
	UPROPERTY(EditAnywhere, Category="Camera FX")
	bool bDestroyImmediately = false;

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UParticleSystemComponent>> SpawnedParticles;
};
