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
	UPROPERTY(EditAnywhere, Category="Hit Check")
	EEquipHand WeaponHand = EEquipHand::Right;
	UPROPERTY(EditAnywhere, Category="Hit Check")
	TArray<FName> TargetSocketNames;
	UPROPERTY(EditAnywhere, Category="Hit Check")
	float SphereSweepRadius = 30.f;
	UPROPERTY(EditAnywhere, Category="Hit Check")
	FGameplayTag EventTag;
	UPROPERTY(EditAnywhere, Category="Hit Check")
	bool bDrawDebug = false;

private:
	TMap<USkeletalMeshComponent*, TSet<AActor*>> HitActorsMap;
	TMap<USkeletalMeshComponent*, TArray<FVector>> PrevSocketLocationMap;

	// 캐릭터에 설정된 무기 메쉬 Get
	class UStaticMeshComponent* GetWeaponMesh(USkeletalMeshComponent* CharacterMesh) const;
};
