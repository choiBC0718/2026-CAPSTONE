// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_PlayWeaponAnim.h"

#include "Component/CAP_WeaponComponent.h"


static TMap<TWeakObjectPtr<USkeletalMeshComponent>, int32> WeaponNotifyCountMap;
static TMap<TWeakObjectPtr<USkeletalMeshComponent>, FSavedWeaponState> WeaponStateMap;

void UAN_PlayWeaponAnim::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;

	UCAP_WeaponComponent* WeaponComp = MeshComp->GetOwner()->FindComponentByClass<UCAP_WeaponComponent>();
	if (!WeaponComp) return;

	USkeletalMeshComponent* TargetWeaponMesh = WeaponComp->GetWeaponMesh(TargetHand);
	if (TargetWeaponMesh)
	{
		// 현재 무기에서 실행 중인 노티파이 개수를 get
		int32& Count = WeaponNotifyCountMap.FindOrAdd(TargetWeaponMesh, 0);
		
		// 처음 시작할 때 최초의 원본 위치 저장
		if (Count == 0)
		{
			FSavedWeaponState State;
			State.OriginalTransform = TargetWeaponMesh->GetRelativeTransform();
			State.OriginalMesh = TargetWeaponMesh->GetSkeletalMeshAsset(); 
	
			WeaponStateMap.Add(TargetWeaponMesh, State);
		}
		// 재생중인 노티파이 수 증가
		Count++;
		
		// 부착 초기화
		TargetWeaponMesh->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CharacterBoneName);

		// 무기 애니메이션 재생
		if (WeaponAnim)
		{
			TargetWeaponMesh->PlayAnimation(WeaponAnim, bLooping);
			TargetWeaponMesh->TickAnimation(0.f, false);
			TargetWeaponMesh->RefreshBoneTransforms();
		}
	}
}

void UAN_PlayWeaponAnim::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp || !MeshComp->GetOwner()) return;
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;

	UCAP_WeaponComponent* WeaponComp = MeshComp->GetOwner()->FindComponentByClass<UCAP_WeaponComponent>();
	if (!WeaponComp) return;

	USkeletalMeshComponent* TargetWeaponMesh = WeaponComp->GetWeaponMesh(TargetHand);
	if (TargetWeaponMesh && AlignBoneName != NAME_None)
	{
		if (FSavedWeaponState* SavedState = WeaponStateMap.Find(TargetWeaponMesh))
		{
			if (SavedState->OriginalMesh.Get() != TargetWeaponMesh->GetSkeletalMeshAsset()) return;
		}
		
		FTransform BoneTransform = TargetWeaponMesh->GetSocketTransform(AlignBoneName, RTS_Component);
		FTransform FinalTransform = BoneTransform.Inverse() * GripOffsetTransform;
		
		TargetWeaponMesh->SetRelativeTransform(FinalTransform);
	}
}

void UAN_PlayWeaponAnim::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!MeshComp || !MeshComp->GetOwner()) return;

	UCAP_WeaponComponent* WeaponComp = MeshComp->GetOwner()->FindComponentByClass<UCAP_WeaponComponent>();
	if (!WeaponComp) return;

	USkeletalMeshComponent* TargetWeaponMesh = WeaponComp->GetWeaponMesh(TargetHand);
	if (TargetWeaponMesh)
	{
		if (int32* CountPtr = WeaponNotifyCountMap.Find(TargetWeaponMesh))
		{
			// 종료 시 노티파이 감소
			(*CountPtr)--;

			// 노티파이 0개 (종료)
			if (*CountPtr <= 0)
			{
				TargetWeaponMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	
				if (FSavedWeaponState* SavedState = WeaponStateMap.Find(TargetWeaponMesh))
				{
					if (SavedState->OriginalMesh.IsValid() && SavedState->OriginalMesh.Get() == TargetWeaponMesh->GetSkeletalMeshAsset())
					{
						TargetWeaponMesh->SetRelativeTransform(SavedState->OriginalTransform);
					}
				}
	
				// 메모리 청소
				WeaponNotifyCountMap.Remove(TargetWeaponMesh);
				WeaponStateMap.Remove(TargetWeaponMesh);
			}
		}
	}
}
