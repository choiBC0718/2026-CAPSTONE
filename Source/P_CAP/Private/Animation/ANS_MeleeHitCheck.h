// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "ANS_MeleeHitCheck.generated.h"

/**
 * 근접 무기 소켓에 따라 트레이스 그리는 Animation Notify State class
 */
UCLASS()
class UANS_MeleeHitCheck : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	UANS_MeleeHitCheck();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

protected:
	/** 무기가 아닌 캐릭터 본 소켓 이용 할건지 (주먹용도) */
	UPROPERTY(EditAnywhere, Category="Hit Check")
	bool bUseCharacterBones = false;
	/** 들고 있는 무기의 손 위치*/
	UPROPERTY(EditAnywhere, Category="Hit Check")
	EEquipHand WeaponHand = EEquipHand::Right;
	/** 무기 메쉬에 추가되있는 소켓 이름 (시작점~끝점) */
	UPROPERTY(EditAnywhere, Category="Hit Check")
	TArray<FName> TargetSocketNames;
	/** 트레이스 두께 */
	UPROPERTY(EditAnywhere, Category="Hit Check")
	float SweepRadius = 30.f;
	UPROPERTY(EditAnywhere, Category="Hit Check")
	float SweepHalfHeight = 60.f;
	/** 이벤트 태그 */
	UPROPERTY(EditAnywhere, Category="Hit Check")
	FGameplayTag EventTag;
	/** 디버그 그리는지 */
	UPROPERTY(EditAnywhere, Category="Hit Check")
	bool bDrawDebug = false;

private:
	// 중복 타격 방지 Map <Key = 공격 캐릭터, value = 타격 액터>
	TMap<USkeletalMeshComponent*, TSet<AActor*>> HitActorsMap;
	// 이전 프레임 위치 기록 Map <Key = 공격 캐릭터, value = 위치>
	TMap<USkeletalMeshComponent*, TArray<FVector>> PrevSocketLocationMap;

	// 캐릭터에 설정된 무기 메쉬 Get
	class USkeletalMeshComponent* GetWeaponMesh(USkeletalMeshComponent* CharacterMesh) const;
};
