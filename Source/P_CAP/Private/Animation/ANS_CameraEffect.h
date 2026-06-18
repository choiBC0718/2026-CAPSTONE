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

	// 화면(카메라) 앞 어느 위치에 띄울지 오프셋 (보통 X축으로 10~20 정도 앞으로 뺍니다)
	UPROPERTY(EditAnywhere, Category="Camera FX")
	FTransform ParticleOffset = FTransform(FRotator::ZeroRotator, FVector(15.f, 0.f, 0.f));

	// 이펙트를 즉시 끌지, 잔여 파티클이 자연스럽게 사라지도록(Deactivate) 둘지 결정
	UPROPERTY(EditAnywhere, Category="Camera FX")
	bool bDestroyImmediately = false;

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UParticleSystemComponent>> SpawnedParticles;
};
