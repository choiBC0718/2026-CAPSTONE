// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_WeaponTrail.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void UANS_WeaponTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;

	if (!MeshComp || !TrailSystem) 
		return;

	// 일체형 스켈레톤이므로 MeshComp에 직접 파티클을 부착합니다.
	UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAttached(
		TrailSystem,
		MeshComp,
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true
	);

	if (ParticleComp)
	{
		// 두 소켓 사이를 연결하는 트레일 생성 시작
		ParticleComp->BeginTrails(BaseSocketName, TipSocketName, ETrailWidthMode_FromCentre, TrailWidth);
		SpawnedTrails.Add(MeshComp, ParticleComp);
	}
}

void UANS_WeaponTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;

	if (!MeshComp) 
		return;

	UParticleSystemComponent** FoundComp = SpawnedTrails.Find(MeshComp);
	if (FoundComp && *FoundComp)
	{
		// 파티클 생성을 중단하고 자연스럽게 소멸하도록 처리
		(*FoundComp)->DeactivateSystem();
		SpawnedTrails.Remove(MeshComp);
	}
}
