// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GAS/ItemBehavior/CAP_ItemBehaviorBase.h"
#include "ItemBehavior_SummonPet.generated.h"

/**
 * 
 */
UCLASS()
class UItemBehavior_SummonPet : public UCAP_ItemBehaviorBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class ACAP_FlyingPetPawn> PetPawnClass;
	UPROPERTY(EditDefaultsOnly)
	class USkeletalMesh* PetSkeletalMesh;
	
	UPROPERTY(EditDefaultsOnly)
	ESkillDamageType DamageType = ESkillDamageType::Magical;
	UPROPERTY(EditDefaultsOnly)
	float BaseDamage = 0.f;
	UPROPERTY(EditDefaultsOnly)
	float DamageMultiplier = 0.f;

	virtual void OnEquipped(class UCAP_ItemInstance* ItemInst, class UAbilitySystemComponent* ASC) const override;
	virtual void OnUnequipped(class UCAP_ItemInstance* ItemInst, class UAbilitySystemComponent* ASC) const override;

protected:
	// 스폰된 펫의 메모리 포인터 관리 (장착 해제 시 파괴하기 위함)
	mutable TWeakObjectPtr<class ACAP_FlyingPetPawn> SpawnedPet;

private:
	// 펫 위치가 겹치지 않도록 할당할 배열
	const TArray<FVector> PetOffsetSlots = {
		FVector(-100.f, -100.f, 150.f), // 1번 펫: 좌측 뒤
		FVector(-100.f, 100.f, 150.f),  // 2번 펫: 우측 뒤
		FVector(-150.f, 0.f, 200.f),    // 3번 펫: 정중앙 더 높은 뒤
		FVector(-100.f, -200.f, 150.f), // 4번 펫: 더 먼 좌측
		FVector(0.f, -150.f, 150.f),	// 5번 펫: 좌측
		FVector(0.f, 150.f, 150.f),		// 6번 펫: 우측
	};
};
