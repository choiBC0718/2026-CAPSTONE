// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CAP_WeaponDataAsset.generated.h"

/** 무기 데이터 에셋에서 무기마다 설정할 Ability 구조체 */
USTRUCT(BlueprintType)
struct FWeaponSkillData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftClassPtr<class UCAP_GameplayAbility> AbilityClass = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* AbilityMontage = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UGameplayEffect> SkillDamageTypeEffect = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamageMultiplier = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float CooldownTime = 1.f;
};

UENUM(BlueprintType)
enum class EEquipHand : uint8
{
	Right			UMETA(DisplayName = "오른손 장착"),
	Left			UMETA(DisplayName = "왼손 장착")
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

	/** 어느 손에 장착시킬지*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEquipHand EquipHand = EEquipHand::Right;
	/** 캐릭터에 부착시킬 무기 스켈레탈 메시*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<class USkeletalMesh> WeaponMesh = nullptr;
	/** 캐릭터 쪽 부착 지점 (본 이름 or 소켓 이름)
	 * 기본적으로 hand_l / hand_r 사용
	 * (특수 위치인 경우 전용 소켓 이름 입력)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CharacterBoneName = FName("hand_r");
	/** 무기 스켈레탈 메시의 뼈 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AlignBoneName = FName("hand_r");
	/** 최종 미세 보정값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform GripOffsetTransform = FTransform();
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
	/** 무기 태생 등급 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	EWeaponGrade DefaultGrade = EWeaponGrade::Normal;
	/**무기 등급 - 조회할 테이블 행 이름 Map
	 * ex.(Normal - DualSword_Normal)
	 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	TMap<EWeaponGrade, FName> GradeDataMap;
	
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TArray<FWeaponVisualInfo> WeaponVisualInfos;
	
	/** 무기의 기본 공격 */
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	FWeaponSkillData BasicAbility;
	/** 무기의 고유 스킬 배열 */
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	TArray<FWeaponSkillData> ActiveAbilityArray;
	
	
	UPROPERTY(EditDefaultsOnly, Category="Animtaion")
	class UAnimSequence* IdleAnim = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Animtaion")
	class UAnimSequence* JogStartAnim = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Animtaion")
	class UAnimSequence* JoggingAnim = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Animtaion")
	class UAnimSequence* JogEndAnim = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Animtaion")
	float JogEndStartTime = 0.f;
};
