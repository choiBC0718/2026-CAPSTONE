// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "CAP_WeaponComponent.generated.h"

// 무기 변경 했다는 것을 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponChanged, class UCAP_WeaponInstance*, NewWeaponInstance, class UCAP_WeaponInstance*, OldWeaponInstance);
// 무기의 스킬 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSkillChanged, class UCAP_WeaponInstance*, WeaponInst);

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

	UPROPERTY()
	FOnWeaponChanged OnWeaponChanged;
	UPROPERTY()
	FOnWeaponSkillChanged OnWeaponSkillChanged;

	const TArray<class UCAP_WeaponInstance*>& GetEquippedWeapons() const {return EquippedWeapons;}
	class USkeletalMeshComponent* GetWeaponMesh(EEquipHand Hand) const;

	// 특정 무기를 가지고 있는지 확인
	bool HasWeapon(class UCAP_WeaponDataAsset* WeaponDA) const;
	// 무기 획득시 능력을 GiveAbility
	void GrantAbilities(class UCAP_WeaponInstance* WeaponInst);
	// 무기 드랍 시에 ClearAbility
	void ClearAbilities(class UCAP_WeaponInstance* WeaponInst);

	struct FWeaponComponentSaveData CreateSaveData() const;
	void RestoreFromSaveData(const struct FWeaponComponentSaveData& InData);
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
	
	void ApplyWeaponData(class UCAP_WeaponInstance* WeaponInstance);
	void AttachWeaponMesh(class UCAP_WeaponDataAsset* WeaponDA);
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	int MaxWeaponCount = 2;

	UPROPERTY()
	class UCAP_AbilitySystemComponent* ASC;
};
