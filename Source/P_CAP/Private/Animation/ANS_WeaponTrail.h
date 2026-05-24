// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "ANS_WeaponTrail.generated.h"

/**
 * 
 */
UCLASS()
class UANS_WeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	TObjectPtr<UParticleSystem> TrailSystem;

	UPROPERTY(EditAnywhere, Category="Trail")
	EEquipHand WeaponHand = EEquipHand::Right;

	UPROPERTY(EditAnywhere, Category="Trail")
	FName BaseSocketName = TEXT("FX_WeaponBase_R");
	
	UPROPERTY(EditAnywhere, Category="Trail")
	FName TipSocketName = TEXT("FX_WeaponTip_R");

	UPROPERTY(EditAnywhere, Category="Trail")
	float TrailWidth = 1.0f;
private:
	TMap<USkeletalMeshComponent*, UParticleSystemComponent*> SpawnedTrails;
	class USkeletalMeshComponent* GetWeaponMesh(USkeletalMeshComponent* CharacterMesh) const;
};
