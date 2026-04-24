// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "CAP_WeaponComponent.generated.h"

// 무기 변경 했다는 것을 알리는 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChanged, class UCAP_WeaponInstance*, NewWeaponInstance);

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

	TArray<class UCAP_WeaponInstance*> GetEquippedWeapons() const {return EquippedWeapons;}
	class USkeletalMeshComponent* GetWeaponMesh(EEquipHand Hand) const;
	
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
	void ClearCurrentWeaponVisuals();
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	int MaxWeaponCount = 2;

	UPROPERTY()
	class UCAP_AbilitySystemComponent* ASC;

	// 무기 획득시 능력을 비활성화로 쥐여줌 (EAbilityInputID = INDEX_NONE)
	void GrantWeaponAbilities(class UCAP_WeaponInstance* WeaponInst);
	// 무기 장착 시 키 연결 (EAbilityInputID)
	void MapWeaponAbilities(class UCAP_WeaponInstance* WeaponInst);
	// 무기 교체 시 사용 안하는 능력 InputID 비활성화
	void UnmapWeaponAbilities(class UCAP_WeaponInstance* WeaponInst);
	// 무기 드랍 시에 ClearAbility
	void RemoveWeaponAbilities(class UCAP_WeaponInstance* WeaponInst);
};
