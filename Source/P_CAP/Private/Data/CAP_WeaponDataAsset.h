// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_WeaponDataAsset.generated.h"


UENUM(BlueprintType)
enum class ESkillLogicType : uint8
{
	Melee			UMETA(DisplayName = "근접 로직"),
	Projectile		UMETA(DisplayName = "투사체 로직"),
	Targeting		UMETA(DisplayName = "타게팅 로직")
};

/** 무기 데이터 에셋에서 무기마다 설정할 Ability 구조체 */
USTRUCT(BlueprintType)
struct FWeaponSkillData : public FTableRowBase
{
	GENERATED_BODY()

	/** 스킬이 사용할 GA 클래스*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic")
	TSoftClassPtr<class UCAP_GameplayAbility> AbilityClass = nullptr;
	/** GA 클래스에서 실행시킬 Task 분기를 위한 구분 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic")
	ESkillLogicType LogicType = ESkillLogicType::Melee;

	// 투사체 클래스 (Targeting 로직에 넣을 시 타게팅 + 투사체로 실행)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic|Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Projectile || LogicType == ESkillLogicType::Targeting"))
	TSoftClassPtr<class ACAP_ProjectileBase> ProjectileClass = nullptr;
	// 투사체를 스폰시킬 무기 에셋의 소켓 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Logic|Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Projectile || LogicType == ESkillLogicType::Targeting"))
	FName ProjectileSocketName = NAME_None;
	// 캐스트 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic|Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	TSoftObjectPtr<class UAnimMontage> CastMontage = nullptr;
	// 타겟 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic|Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	TSoftClassPtr<class ACAP_TargetActor> TargetActorClass = nullptr;
	// 타게팅 사거리 인디케이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic|Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	TSoftClassPtr<class ACAP_TargetRangeIndicator> RangeIndicatorClass = nullptr;
	// 타게팅 사거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic|Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	float MaxTargetingRange = 1000.f;
	// 데미지 구역 크기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic|Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	float TargetAreaRadius = 300.f;
	
	/** 스킬 애니메이션 몽타주*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
	TSoftObjectPtr<class UAnimMontage> AbilityMontage = nullptr;

	// 게임플레이 큐 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data")
	FGameplayTag GameplayCueTag;
	/** 해당 스킬에 사용할 데미지 (물리/마법/방어력) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data")
	TSoftClassPtr<class UGameplayEffect> SkillDamageTypeEffect = nullptr;
	/** 스킬 데미지 배수*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data")
	float BaseDamageMultiplier = 1.f;
	/** 스킬 쿨타임*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skill Data")
	float CooldownTime = 1.f;
	
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
	/** 무기 아이콘 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	TSoftObjectPtr<class UTexture2D> WeaponIcon;
	/** 무기 태생 등급 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	EItemGrade DefaultGrade = EItemGrade::Normal;
	/** 무기 설명*/
	UPROPERTY(EditDefaultsOnly, Category="Data", meta=(MultiLine="true"))
	FText Description;
	/**무기 등급 - 조회할 테이블 행 이름 Map
	 * ex.(Normal - DualSword_Normal)
	 */
	UPROPERTY(EditDefaultsOnly, Category="Data")
	TMap<EItemGrade, FName> GradeDataMap;
	
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
