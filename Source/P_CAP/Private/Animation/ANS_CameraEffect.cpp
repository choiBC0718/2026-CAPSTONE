// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_CameraEffect.h"

#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


void UANS_CameraEffect::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                    const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp || !CameraParticle) return;

	APawn* OwnerPawn = Cast<APawn>(MeshComp->GetOwner());
	if (OwnerPawn)
	{
		UCameraComponent* CameraComp = OwnerPawn->FindComponentByClass<UCameraComponent>();
		if (CameraComp)
		{
			UParticleSystemComponent* SpawnedPSC = UGameplayStatics::SpawnEmitterAttached(
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
			
			if (SpawnedPSC)
			{
				SpawnedParticles.Add(MeshComp, SpawnedPSC);
			}
		}
	}
}

void UANS_CameraEffect::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!MeshComp) return;
	
	if (TWeakObjectPtr<UParticleSystemComponent>* FoundPSC = SpawnedParticles.Find(MeshComp))
	{
		if (FoundPSC->IsValid())
		{
			if (bDestroyImmediately)
			{
				(*FoundPSC)->DestroyComponent(); 
			}
			else
			{
				(*FoundPSC)->Deactivate(); 
			}
		}
		SpawnedParticles.Remove(MeshComp);
	}
}
