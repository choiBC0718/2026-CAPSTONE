// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "AN_PlayWeaponAnim.generated.h"

/**
 * 
 */
UCLASS()
class UAN_PlayWeaponAnim : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, Category = "Weapon")
	EEquipHand TargetHand = EEquipHand::Left;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	class UAnimSequence* WeaponAnim;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bLooping = false;

	// 1. 캐릭터의 어느 본에 붙일지
	UPROPERTY(EditAnywhere, Category = "Alignment")
	FName CharacterBoneName = FName("hand_l");

	// 2. 무기 스켈레탈 메시의 어떤 본을 기준으로 할지
	UPROPERTY(EditAnywhere, Category = "Alignment")
	FName AlignBoneName = FName("bow_base");

	// 3. 디테일한 미세 조정 트랜스폼
	UPROPERTY(EditAnywhere, Category = "Alignment")
	FTransform GripOffsetTransform;
};
