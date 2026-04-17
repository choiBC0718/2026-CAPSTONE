// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_PlayWeaponAnim.h"

#include "Items/Weapon/CAP_WeaponComponent.h"


static TMap<TWeakObjectPtr<USkeletalMeshComponent>, int32> WeaponNotifyCountMap;
static TMap<TWeakObjectPtr<USkeletalMeshComponent>, FTransform> WeaponOriginalTransformMap;

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
		// ✨ 1. 현재 무기에서 실행 중인 노티파이 개수를 가져옵니다 (없으면 0)
		int32& Count = WeaponNotifyCountMap.FindOrAdd(TargetWeaponMesh, 0);
		
		// ✨ 2. 처음(콤보 1) 시작할 때 딱 한 번만 완벽한 원본 위치를 저장합니다!
		if (Count == 0)
		{
			WeaponOriginalTransformMap.Add(TargetWeaponMesh, TargetWeaponMesh->GetRelativeTransform());
		}
		
		// 노티파이 개수 증가 (콤보 2가 시작되면 Count가 2가 됨)
		Count++;
		
		// 부착 초기화 (DA에서 하던 것과 동일하게 손에 딱 붙입니다)
		TargetWeaponMesh->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, CharacterBoneName);

		// 무기 애니메이션 재생 시작!
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
		// ✨ 여기가 핵심입니다! ✨
		// 애니메이션이 재생되면서 고스트 팔이 움직이면, 이 BoneTransform 값이 매 프레임 바뀝니다.
		FTransform BoneTransform = TargetWeaponMesh->GetSocketTransform(AlignBoneName, RTS_Component);
		
		// 유저님이 고안하신 천재적인 역연산(Inverse)을 매 프레임 업데이트해버립니다.
		FTransform FinalTransform = BoneTransform.Inverse() * GripOffsetTransform;

		// 고스트 팔이 뻗어나가는 만큼 컴포넌트를 반대 방향으로 밀어넣어서 완벽하게 손에 고정시킵니다!
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
			// ✨ 노티파이가 끝날 때마다 개수를 줄입니다.
			(*CountPtr)--;

			// ✨ 개수가 0 이하가 되었다 = 콤보가 완전히 끝났다! (이때만 복구)
			if (*CountPtr <= 0)
			{
				// 애니메이션 강제 정지
				TargetWeaponMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
				
				// 저장해둔 진짜 원본 위치로 복구
				if (FTransform* SavedTransform = WeaponOriginalTransformMap.Find(TargetWeaponMesh))
				{
					TargetWeaponMesh->SetRelativeTransform(*SavedTransform);
				}
				
				// 다 썼으니 메모리 청소
				WeaponNotifyCountMap.Remove(TargetWeaponMesh);
				WeaponOriginalTransformMap.Remove(TargetWeaponMesh);
			}
		}
	}
}
