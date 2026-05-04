// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_WeaponInstance.h"
#include "Interactables/CAP_InteractableBase.h"
#include "CAP_WorldWeapon.generated.h"

/*
 *	월드에 스폰되어 상호작용 가능한 무기 액터
 */
UCLASS()
class ACAP_WorldWeapon : public ACAP_InteractableBase
{
	GENERATED_BODY()
	
public:	
	ACAP_WorldWeapon();
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void Interact(class AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon Data")
	class UCAP_WeaponDataAsset* WeaponDA;
	
	UPROPERTY(BlueprintReadWrite,Category="Weapon Data", meta=(ExposeOnSpawn="true"))
	class UCAP_WeaponInstance* WeaponInstance;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USceneComponent* MeshContainer;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USkeletalMeshComponent* WeaponMesh_R;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USkeletalMeshComponent* WeaponMesh_L;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class URotatingMovementComponent* RotatingMovement;
};
