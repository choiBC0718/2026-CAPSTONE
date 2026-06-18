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
	
	// 로컬 플레이어인지 확인 (다른 사람이 스킬 쓴다고 내 화면에 피가 튀지 않게)
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		// 1. 캐릭터가 가지고 있는 카메라 컴포넌트를 찾음
		UCameraComponent* CameraComp = OwnerPawn->FindComponentByClass<UCameraComponent>();
		if (CameraComp)
		{
			// 2. 카메라 렌즈(컴포넌트)에 파티클을 직접 부착하여 스폰
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

			// 3. 나중에 지울 수 있도록 맵에 등록
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

	// 맵에서 해당 메시가 띄웠던 파티클 컴포넌트를 찾음
	if (TWeakObjectPtr<UParticleSystemComponent>* FoundPSC = SpawnedParticles.Find(MeshComp))
	{
		if (FoundPSC->IsValid())
		{
			if (bDestroyImmediately)
			{
				// 이펙트를 뚝 끊어버림
				(*FoundPSC)->DestroyComponent(); 
			}
			else
			{
				// 스폰을 멈추고 화면에 묻은 피 등은 수명에 맞춰 자연스럽게 사라지도록 함
				(*FoundPSC)->Deactivate(); 
			}
		}

		// 추적이 끝났으므로 맵에서 제거하여 찌꺼기(메모리 누수) 방지
		SpawnedParticles.Remove(MeshComp);
	}
}
