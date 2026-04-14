// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_HitBox.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


void UAN_HitBox::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	HitActorsMap.Add(MeshComp, TSet<AActor*>());

	if (!MeshComp) return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor) return;

	UWorld* World = MeshComp->GetWorld();
	if (!World) return;

	// 1. 소켓(또는 Root)을 기준으로 한 실제 월드 위치/회전 계산
	FTransform SocketTransform = MeshComp->GetSocketTransform(SocketName);
	FVector WorldLocation = SocketTransform.TransformPosition(LocalOffset);
	FRotator WorldRotation = (SocketTransform.GetRotation() * LocalRotation.Quaternion()).Rotator();

	// 에디터 프리뷰 모드
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

					// 1. 무기의 현재 뼈(소켓) 위치를 가져옵니다.
					FVector ExactWeaponLoc = MeshComp->GetSocketLocation(SocketName);
					
					// 2. 무기에서 적을 향하는 방향 (피가 튀거나 스파크가 튈 방향)
					FVector DirToTarget = (HitActor->GetActorLocation() - ExactWeaponLoc).GetSafeNormal();

					FHitResult DummyHit(HitActor, nullptr, ExactWeaponLoc, DirToTarget);
					DummyHit.ImpactPoint = ExactWeaponLoc;
					DummyHit.ImpactNormal = DirToTarget * -1.f;

					FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(DummyHit);
					Payload.TargetData.Add(TargetData);

					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
				}
			}
		}
	}
	
	if (MeshComp)
	{
		HitActorsMap.Remove(MeshComp);
	}
}

void UAN_HitBox::DrawDebugShape(UWorld* World, const FVector& Location, const FRotator& Rotation) const
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
