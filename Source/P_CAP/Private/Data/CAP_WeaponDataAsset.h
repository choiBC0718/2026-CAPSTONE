// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_ItemDataAsset.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "InstancedStruct.h"
#include "CAP_WeaponDataAsset.generated.h"


USTRUCT(BlueprintType)
struct FSkillLogicDataBase
{
	GENERATED_BODY()
	virtual ~FSkillLogicDataBase() = default;

	virtual int32 GetNumOfProjectiles() const {return 1;}
	virtual int32 GetMaxHitCount() const {return 1;}
	virtual float GetSpreadAngle() const {return 0.f;}
	virtual TSoftClassPtr<class ACAP_ProjectileBase> GetProjectileClass() const {return nullptr;}
};
// 투사체 전용 데이터
USTRUCT(BlueprintType)
struct FProjectileLogicData: public FSkillLogicDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class ACAP_ProjectileBase> ProjectileClass = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 NumOfProjectiles = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 MaxHitCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(EditCondition = "NumOfProjectiles > 1"))
	float SpreadAngle = 30.f;

	virtual int32 GetNumOfProjectiles() const override {return NumOfProjectiles;}
	virtual int32 GetMaxHitCount() const override {return MaxHitCount;}
	virtual float GetSpreadAngle() const override {return SpreadAngle;}
	virtual TSoftClassPtr<class ACAP_ProjectileBase> GetProjectileClass() const override {return ProjectileClass;}
};
// 타게팅 전용 데이터
USTRUCT(BlueprintType)
struct FTargetingLogicData : public FSkillLogicDataBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<class UAnimMontage> CastMontage = nullptr;
	// 스킬이 시전될 타겟 액터
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class ACAP_TargetActor> TargetActorClass = nullptr;
	// 스킬 시전 가능한 최대 사거리 인디케이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class ACAP_TargetRangeIndicator> RangeIndicatorClass = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxTargetingRange = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TargetAreaRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<class ACAP_ProjectileBase> ProjectileClass = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
	float FallingSpawnHeight = 1000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ProjectileSocketName = NAME_None;
	/*
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 NumOfProjectiles = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 MaxHitCount = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpreadAngle = 30.f;

	virtual int32 GetNumOfProjectiles() const override {return NumOfProjectiles;}
	virtual int32 GetMaxHitCount() const override {return MaxHitCount;}
	virtual float GetSpreadAngle() const override {return SpreadAngle;}
	*/
	virtual TSoftClassPtr<class ACAP_ProjectileBase> GetProjectileClass() const override {return ProjectileClass;}
};

UENUM(BlueprintType)
enum class ESkillLogicType : uint8
{
	Melee			UMETA(DisplayName = "근접 로직"),
	Projectile		UMETA(DisplayName = "투사체 로직"),
	Targeting		UMETA(DisplayName = "타게팅 로직")
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

	/** 스킬이 사용할 GA 클래스*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic")
	TSoftClassPtr<class UCAP_GameplayAbility> AbilityClass = nullptr;
	/** GA 클래스에서 실행시킬 Task 분기를 위한 구분 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic")
	ESkillLogicType LogicType = ESkillLogicType::Melee;
	
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


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Logic", meta=(BaseStruct="/Script/P_CAP.SkillLogicDataBase", ExcludeBaseStruct))
	FInstancedStruct LogicData;
	/*
	// 투사체 클래스 (Targeting 로직에 넣을 시 타게팅 + 투사체로 실행)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Projectile || LogicType == ESkillLogicType::Targeting"))
	TSoftClassPtr<class ACAP_ProjectileBase> ProjectileClass = nullptr;
	// 투사체를 스폰시킬 무기 에셋의 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs", meta = (EditCondition = "LogicType == ESkillLogicType::Projectile || LogicType == ESkillLogicType::Targeting"))
	FName ProjectileSocketName = NAME_None;
	
	// 캐스트 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs|Targeting", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	TSoftObjectPtr<class UAnimMontage> CastMontage = nullptr;
	// 타겟 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs|Targeting", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	TSoftClassPtr<class ACAP_TargetActor> TargetActorClass = nullptr;
	// 타게팅 사거리 인디케이터
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs|Targeting", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	TSoftClassPtr<class ACAP_TargetRangeIndicator> RangeIndicatorClass = nullptr;
	// 타게팅 사거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs|Targeting", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	float MaxTargetingRange = 1000.f;
	// 데미지 구역 크기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs|Targeting", meta = (EditCondition = "LogicType == ESkillLogicType::Targeting"))
	float TargetAreaRadius = 300.f;
	
	// 투사체 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs|Projectile", meta = (EditCondition = "LogicType == ESkillLogicType::Projectile"))
	int32 NumOfProjectiles = 1;
	// 투사체 퍼짐 각도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Needs|Projectile", meta = (EditCondition = "LogicType == ESkillLogicType::Projectile && NumOfProjectiles>=2"))
	float SpreadAngle = 30.f;
	*/
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
