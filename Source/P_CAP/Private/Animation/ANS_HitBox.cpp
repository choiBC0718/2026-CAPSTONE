// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_HitBox.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void UANS_HitBox::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp) return;

	HitActorsMap.Add(MeshComp, TSet<AActor*>());
}

void UANS_HitBox::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	// 1. 소켓(또는 Root)을 기준으로 한 실제 월드 위치/회전 계산
	FTransform SocketTransform = MeshComp->GetSocketTransform(SocketName);
	FVector WorldLocation = SocketTransform.TransformPosition(LocalOffset);
	FRotator WorldRotation = (SocketTransform.GetRotation() * LocalRotation.Quaternion()).Rotator();

	// 2. 🌟 에디터 프리뷰 모드: 애니메이션 창에서 몽타주를 드래그할 때 실시간으로 그려줍니다!
	if (bDrawDebug)
	{
		DrawDebugShape(World, WorldLocation, WorldRotation);
	}

	// 3. 실제 타격 판정 (Overlap)
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(OwnerActor);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> OverlappedActors;

	// 모양에 따라 다른 함수 호출
	switch (ShapeType)
	{
		case EHitboxShape::Box:
			UKismetSystemLibrary::BoxOverlapActors(World, WorldLocation, BoxExtent, ObjectTypes, nullptr, IgnoreActors, OverlappedActors);
			break;
		case EHitboxShape::Sphere:
			UKismetSystemLibrary::SphereOverlapActors(World, WorldLocation, SphereRadius, ObjectTypes, nullptr, IgnoreActors, OverlappedActors);
			break;
		case EHitboxShape::Capsule:
			UKismetSystemLibrary::CapsuleOverlapActors(World, WorldLocation, CapsuleRadius, CapsuleHalfHeight, ObjectTypes, nullptr, IgnoreActors, OverlappedActors);
			break;
	}

	// 4. 적중 처리 및 데미지 이벤트 발송
	TSet<AActor*>* HitActors = HitActorsMap.Find(MeshComp);
	if (!HitActors) return;

	// ASC가 있는지 확인 (에디터 프리뷰 뷰포트 에러 방지)
	bool bHasASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor) != nullptr;

	for (AActor* HitActor : OverlappedActors)
	{
		if (HitActor && !HitActors->Contains(HitActor))
		{
			HitActors->Add(HitActor);

			if (bHasASC)
			{
				for (int32 i = 0; i < EventFireCount; ++i)
				{
					FGameplayEventData Payload;
					Payload.Instigator = OwnerActor;
					Payload.Target = HitActor;

					// ✨ 1. 무기의 현재 뼈(소켓) 위치를 가져옵니다. (칼날의 위치)
					FVector ExactWeaponLoc = MeshComp->GetSocketLocation(SocketName);
					
					// ✨ 2. 무기에서 적을 향하는 방향을 구합니다. (피가 튀거나 스파크가 튈 방향)
					FVector DirToTarget = (HitActor->GetActorLocation() - ExactWeaponLoc).GetSafeNormal();

					// ✨ 3. 가짜 HitResult를 만들고, 이펙트가 터질 위치를 '칼날 위치'로 강제로 꽂아 넣습니다!
					FHitResult DummyHit(HitActor, nullptr, ExactWeaponLoc, DirToTarget);
					DummyHit.ImpactPoint = ExactWeaponLoc; // 🌟 핵심: GameplayCue는 이 좌표를 보고 이펙트를 띄웁니다!
					DummyHit.ImpactNormal = DirToTarget * -1.f; // 이펙트가 무기 반대 방향으로 튀게 설정

					FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(DummyHit);
					Payload.TargetData.Add(TargetData);

					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
				}
			}
		}
	}
}

void UANS_HitBox::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp)
	{
		HitActorsMap.Remove(MeshComp);
	}
}

void UANS_HitBox::DrawDebugShape(UWorld* World, const FVector& Location, const FRotator& Rotation) const
{
	float LifeTime = 2.f; 

	switch (ShapeType)
	{
	case EHitboxShape::Box:
		DrawDebugBox(World, Location, BoxExtent, Rotation.Quaternion(), DebugColor, false, LifeTime, 0, 2.f);
		break;
	case EHitboxShape::Sphere:
		DrawDebugSphere(World, Location, SphereRadius, 24, DebugColor, false, LifeTime, 0, 2.f);
		break;
	case EHitboxShape::Capsule:
		DrawDebugCapsule(World, Location, CapsuleHalfHeight, CapsuleRadius, Rotation.Quaternion(), DebugColor, false, LifeTime, 0, 2.f);
		break;
	}
}
