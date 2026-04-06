// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/ActorComponent.h"
#include "CAP_WeaponComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCAP_WeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCAP_WeaponComponent();
	virtual void BeginPlay() override;

	void PickupWeapon(class UCAP_WeaponInstance* NewWeaponInst);
	void SwapWeapon();
	class UCAP_WeaponInstance* GetCurrentWeaponInstance() const;

private:
	UPROPERTY()
	class USkeletalMeshComponent* WeaponMesh_L;
	UPROPERTY()
	class USkeletalMeshComponent* WeaponMesh_R;

	UPROPERTY()
	TArray<class UCAP_WeaponInstance*> EquippedWeapons;
	int32 CurrentWeaponIndex=0;
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	class UCAP_WeaponDataAsset* DefaultBasicWeapon;
	
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> CurrentWeaponAbilityHandles;
	
	void ApplyWeaponData(class UCAP_WeaponInstance* WeaponInstance);
	void ClearCurrentWeaponData();
};
