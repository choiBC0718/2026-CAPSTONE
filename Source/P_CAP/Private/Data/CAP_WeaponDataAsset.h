// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CAP_WeaponDataAsset.generated.h"

UENUM(BlueprintType)
enum class EWeaponAnimType : uint8
{
	Unarmed			UMETA(DisplayName = "맨손"),
	Melee_1H		UMETA(DisplayName = "한손 근접"),
	Melee_2H		UMETA(DisplayName = "양손 근접"),
	Ranged			UMETA(DisplayName = "원거리"),
	Shield			UMETA(DisplayName = "방패"),
};
UENUM(BlueprintType)
enum class EEquipHand : uint8
{
	Right			UMETA(DisplayName = "오른손"),
	Left			UMETA(DisplayName = "왼손")
};
UENUM(BlueprintType)
enum class EWeaponGrade : uint8
{
	Normal			UMETA(DisplayName = "일반"),
	Rare			UMETA(DisplayName = "레어"),
	Epic			UMETA(DisplayName = "에픽"),
	Legendary		UMETA(DisplayName = "레전더리")
};

USTRUCT(BlueprintType)
struct FWeaponVisualInfo
{
	GENERATED_BODY()

	/**캐릭터에 부착시킬 메시*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMesh* WeaponMesh = nullptr;
	/**어느 소켓에 부착시킬 지*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EquipSocketName = NAME_None;
	/**부착 Transform*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform EquipTransform = FTransform();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEquipHand EquipHand = EEquipHand::Right;
};

/**
 * 
 */
UCLASS()
class UCAP_WeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 무기 이름 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	FText WeaponName;
	/** 무기 기본 등급 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	EWeaponGrade DefaultGrade = EWeaponGrade::Normal;
	/**무기 등급 - 조회할 테이블 행 이름 Map
	 * ex.(Normal - DualSword_Normal)
	 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	TMap<EWeaponGrade, FName> GradeDataMap;

	/** 장착 할 무기 액터 클래스 */
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TSubclassOf<class ACAP_WeaponBase> WeaponClass;
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TArray<FWeaponVisualInfo> WeaponVisualInfos;

	/** 무기의 기본 공격 */
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	TSoftClassPtr<class UCAP_GameplayAbility> BasicAttack;
	/** 무기의 고유 스킬 배열 */
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	TArray<TSoftClassPtr<class UCAP_GameplayAbility>> ActiveSkillArray;

	/** 해당 무기 장착 시 애니메이션 상태*/
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	EWeaponAnimType WeaponAnimType = EWeaponAnimType::Unarmed;
};
