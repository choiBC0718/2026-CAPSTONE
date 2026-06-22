// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_CameraEffect.h"

#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

void UAN_CameraEffect::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp || !CameraParticle) return;

	APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
	if (OwnerPawn)
	{
		UCameraComponent* CameraComp = OwnerPawn->FindComponentByClass<UCameraComponent>();
		if (CameraComp)
		{
			UGameplayStatics::SpawnEmitterAttached(
				CameraParticle,
				CameraComp,
				NAME_None,
				ParticleOffset.GetLocation(),
				ParticleOffset.GetRotation().Rotator(),
				ParticleOffset.GetScale3D(),
				EAttachLocation::KeepRelativeOffset,
				true,
				EPSCPoolMethod::None,
				true
			);
		}
	}
}
