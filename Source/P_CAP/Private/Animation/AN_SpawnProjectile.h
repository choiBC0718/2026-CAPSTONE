// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "AN_SpawnProjectile.generated.h"

UENUM(BlueprintType)
enum class ESpawnMeshTarget : uint8
{
	Weapon,
	Character,
};

/**
 * 
 */
UCLASS()
class UAN_SpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SpawnProjectile();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	
	UPROPERTY(EditAnywhere, Category="Event", meta=(Categories="Ability.Event"))
	FGameplayTag EventTag;

	// 캐릭터 스켈레탈 소켓 사용할지, 무기 스켈레탈의 소켓 사용할지
	UPROPERTY(EditAnywhere, Category="Event")
	ESpawnMeshTarget MeshTarget = ESpawnMeshTarget::Weapon;
	UPROPERTY(EditAnywhere, Category="Event", meta=(EditCondition="MeshTarget==ESpawnMeshTarget::Weapon"))
	EEquipHand SpawnHand = EEquipHand::Right;
	UPROPERTY(EditAnywhere, Category="Event")
	FName SocketName = TEXT("Muzzle");
};
