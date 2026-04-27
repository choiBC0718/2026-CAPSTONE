// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_WeaponInstance.h"
#include "Items/CAP_DropItemBase.h"
#include "CAP_WeaponBase.generated.h"

UCLASS()
class ACAP_WeaponBase : public ACAP_DropItemBase
{
	GENERATED_BODY()
	
public:	
	ACAP_WeaponBase();
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter) override;
	virtual void InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter) override;
	virtual UObject* GetInteractData() const override {return WeaponInstance;}
	
	UPROPERTY(EditDefaultsOnly, Category="Weapon Data")
	class UCAP_WeaponDataAsset* WeaponDA;
	
	UPROPERTY(BlueprintReadWrite,Category="Weapon Data", meta=(ExposeOnSpawn="true"))
	class UCAP_WeaponInstance* WeaponInstance;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USkeletalMeshComponent* WeaponMesh_R;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USkeletalMeshComponent* WeaponMesh_L;

};
