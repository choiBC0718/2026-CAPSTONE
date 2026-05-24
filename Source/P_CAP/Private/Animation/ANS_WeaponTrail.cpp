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

	USkeletalMeshComponent* WeaponMesh = GetWeaponMesh(MeshComp);
	if (WeaponMesh)
	{
		UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAttached(
			TrailSystem,
			WeaponMesh,
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

class USkeletalMeshComponent* UANS_WeaponTrail::GetWeaponMesh(USkeletalMeshComponent* CharacterMesh) const
{
	if (!CharacterMesh || !CharacterMesh->GetOwner())
		return nullptr;

	TArray<USkeletalMeshComponent*> WeaponMeshes;
	CharacterMesh->GetOwner()->GetComponents<USkeletalMeshComponent>(WeaponMeshes);
	
	FName TagToFind = (WeaponHand == EEquipHand::Right) ? TEXT("RightHand") : TEXT("LeftHand");
	for (USkeletalMeshComponent* Mesh : WeaponMeshes)
	{
		if (Mesh->ComponentHasTag(TagToFind))
		{
			return Mesh;
		}
	}
	return nullptr;
}
