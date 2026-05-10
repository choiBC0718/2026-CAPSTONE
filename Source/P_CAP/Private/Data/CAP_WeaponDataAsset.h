// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_ItemDataAsset.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_WeaponDataAsset.generated.h"



UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
	None		UMETA(DisplayName = "None"),
	Freeze		UMETA(DisplayName = "Freeze"),
	Bleed		UMETA(DisplayName = "Bleed"),
	Burn		UMETA(DisplayName = "Burn"),
	Stun		UMETA(DisplayName = "Stun"),
};

UENUM(BlueprintType)
enum class ESkillDamageType : uint8
{
	Physical,
	Magical,
};

/** 무기 데이터 에셋에서 무기마다 설정할 Ability 구조체 */
USTRUCT(BlueprintType)
struct FWeaponSkillData : public FTableRowBase
{
	GENERATED_BODY()

	/** 스킬이 사용할 입력 로직 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic")
	TSubclassOf<class UGA_FlowBase> InputAbilityClass = nullptr;
	/** 스킬의 결과물 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic")
	TArray<TSubclassOf<class UGA_PayloadBase>> PayloadAbilityClass;
	
	/** 스킬 애니메이션 몽타주*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TSoftObjectPtr<class UAnimMontage> AbilityMontage = nullptr;

	// 타격 대상에게 적용시킬 이펙트 큐 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data", meta=(Categories="GameplayCue.Hit"))
	FGameplayTag GameplayCueTag;
	/** 스킬 데미지 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data")
	ESkillDamageType DamageType = ESkillDamageType::Physical;
	/** 스킬이 보장할 기본 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data")
	float BaseDamage =0.f;
	/** 스킬 데미지 배수*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data")
	float DamageMultiplier = 1.f;
	/** 스킬 쿨타임*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data")
	float CooldownTime = 1.f;
	/** 스킬 쿨타임*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data", meta=(Categories="Ability.Cooldown"))
	FGameplayTag CooldownTag;
	
	/** 스킬 이름*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill UI")
	FName SkillName = NAME_None;
	/** 스킬 설명*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill UI", meta=(MultiLine=true))
	FText Description;
	/** 스킬 아이콘*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill UI")
	TSoftObjectPtr<class UTexture2D> SkillIcon = nullptr;
};

UENUM(BlueprintType)
enum class EEquipHand : uint8
{
	Right			UMETA(DisplayName = "오른손 장착"),
	Left			UMETA(DisplayName = "왼손 장착")
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
	/** 월드 스폰 시 적용시킬 위치+회전+스케일 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform ConstructionOffset = FTransform();
};

/**
 * 
 */
UCLASS()
class UCAP_WeaponDataAsset : public UCAP_ItemDataBase
{
	GENERATED_BODY()

public:
	/**무기 등급 - 조회할 테이블 행 이름 Map
	 * ex.(Normal - DualSword_Normal)
	 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	TMap<EItemGrade, FDataTableRowHandle> GradeDataMap;
	
	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TArray<FWeaponVisualInfo> WeaponVisualInfos;
	
	/** 무기의 기본 공격 */
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta=(RowType="/Script/P_CAP.WeaponSkillData"))
	FDataTableRowHandle BasicAbility;
	/** 무기의 고유 스킬 배열 */
	UPROPERTY(EditDefaultsOnly, Category="Ability", meta=(RowType="/Script/P_CAP.WeaponSkillData"))
	TArray<FDataTableRowHandle> ActiveAbilityArray;
	
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
