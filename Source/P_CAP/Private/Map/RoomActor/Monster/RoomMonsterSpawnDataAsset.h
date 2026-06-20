// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameFramework/Character.h"
#include "Map/RoomTypes.h"
#include "RoomMonsterSpawnDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FRoomMonsterSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(DisplayName="몬스터 클래스", ToolTip="이 스폰 풀에서 등장할 몬스터 블루프린트 클래스입니다."))
	TSubclassOf<ACharacter> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(ClampMin="1", DisplayName="비용", ToolTip="이 몬스터 1마리를 뽑을 때 사용하는 점수 비용입니다."))
	int32 Cost = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(ClampMin="1", DisplayName="가중치", ToolTip="같은 조건에서 이 몬스터가 선택될 상대 확률입니다. 값이 클수록 더 자주 선택됩니다."))
	int32 Weight = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(ClampMin="1", DisplayName="최대 선택 수", ToolTip="한 번의 스폰 계산에서 이 몬스터가 선택될 수 있는 최대 수입니다."))
	int32 MaxCount = 99;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster", meta=(ClampMin="1", DisplayName="셀당 최대 수", ToolTip="같은 스폰 셀에 이 몬스터가 함께 배치될 수 있는 최대 수입니다."))
	int32 MaxMonstersPerCell = 1;
};

UENUM(BlueprintType)
enum class ERoomReinforcementTrigger : uint8
{
	WhenAliveMonsterCountBelow UMETA(DisplayName="생존 몬스터 수 기준"),
	AfterCombatTime UMETA(DisplayName="전투 시간 기준")
};

USTRUCT(BlueprintType)
struct FRoomReinforcementRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(DisplayName="증원 조건", ToolTip="이 증원이 언제 발동될지 정합니다."))
	ERoomReinforcementTrigger Trigger = ERoomReinforcementTrigger::WhenAliveMonsterCountBelow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0", DisplayName="증원 점수 범위", ToolTip="증원으로 추가 스폰할 몬스터 총점 범위입니다. 기존 Monster Pool에서 이 점수만큼 몬스터를 뽑습니다."))
	FIntPoint ScoreRange = FIntPoint(4, 6);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0.0", DisplayName="조건 만족 후 지연", ToolTip="증원 조건이 만족된 뒤 실제 몬스터와 VFX가 스폰되기까지 기다리는 시간입니다."))
	float DelayAfterTriggered = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0.0", DisplayName="스폰 후 AI 활성화 지연", ToolTip="몬스터와 VFX가 스폰된 뒤 AI가 활성화되기까지 기다리는 시간입니다. 연출을 보여줄 시간을 줄 때 사용합니다."))
	float ActivationDelayAfterSpawn = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0", DisplayName="생존 몬스터 기준 수", ToolTip="증원 조건이 '생존 몬스터 수 기준'일 때 사용합니다. 현재 살아있는 몬스터 수가 이 값 이하가 되면 증원이 발동됩니다."))
	int32 AliveMonsterThreshold = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(ClampMin="0.0", DisplayName="전투 시간", ToolTip="증원 조건이 '전투 시간 기준'일 때 사용합니다. 방 전투 시작 후 이 시간이 지나면 증원이 발동됩니다."))
	float CombatTime = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement", meta=(DisplayName="방 클리어 필수 증원", ToolTip="체크하면 이 증원이 실행되어야 방이 클리어될 수 있습니다. 체크 해제하면 전투가 먼저 끝났을 때 스킵될 수 있는 선택 증원으로 사용합니다."))
	bool bRequiredForRoomClear = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement|VFX", meta=(DisplayName="스폰 VFX", ToolTip="증원 몬스터가 스폰되는 위치에 동시에 재생할 Niagara 이펙트입니다."))
	TObjectPtr<class UNiagaraSystem> SpawnVFX;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement|VFX", meta=(DisplayName="스폰 VFX 위치 보정", ToolTip="스폰 위치 기준으로 VFX를 얼마나 이동해서 재생할지 정합니다. 바닥 높이나 중심 위치 조정에 사용합니다."))
	FVector SpawnVFXOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement|VFX", meta=(DisplayName="스폰 VFX 크기", ToolTip="스폰 VFX의 크기 배율입니다."))
	FVector SpawnVFXScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reinforcement|VFX", meta=(DisplayName="스폰 사운드", ToolTip="증원 몬스터가 스폰되는 위치에서 재생할 사운드입니다."))
	TObjectPtr<class USoundBase> SpawnSound;
};

USTRUCT(BlueprintType)
struct FRoomMonsterSpawnRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room", meta=(DisplayName="방 타입", ToolTip="이 스폰 규칙이 적용될 방 타입입니다."))
	ERoomType RoomType = ERoomType::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room", meta=(ClampMin="0", DisplayName="초기 스폰 점수 범위", ToolTip="방에 처음 입장했을 때 스폰할 몬스터 총점 범위입니다. 기존 초기 스폰에만 사용됩니다."))
	FIntPoint ScoreRange = FIntPoint(10, 20);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room", meta=(DisplayName="몬스터 풀", ToolTip="초기 스폰과 증원 스폰이 공통으로 사용하는 몬스터 후보 목록입니다."))
	TArray<FRoomMonsterSpawnEntry> MonsterPool;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Reinforcement", meta=(DisplayName="증원 목록", ToolTip="전투 중 추가로 등장할 증원 규칙 목록입니다. 초기 스폰에는 사용되지 않습니다."))
	TArray<FRoomReinforcementRule> Reinforcements;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Reinforcement", meta=(ClampMin="0", DisplayName="최대 증원 횟수", ToolTip="이 방에서 증원이 최대 몇 번까지 실행될 수 있는지 정합니다."))
	int32 MaxReinforcementCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Reinforcement", meta=(ClampMin="0", DisplayName="최대 증원 점수", ToolTip="이 방에서 증원으로 사용할 수 있는 총 점수 상한입니다. 증원 점수 범위의 최댓값 기준으로 계산됩니다."))
	int32 MaxReinforcementScore = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Room|Reinforcement", meta=(ClampMin="0.0", DisplayName="증원 최소 간격", ToolTip="증원과 다음 증원 사이에 필요한 최소 시간입니다. 증원이 2번 이상 가능할 때 의미가 있습니다."))
	float MinReinforcementInterval = 3.f;
};

UCLASS(BlueprintType)
class URoomMonsterSpawnDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	const FRoomMonsterSpawnRule* FindRule(ERoomType RoomType) const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster Spawn", meta=(AllowPrivateAccess="true"))
	TArray<FRoomMonsterSpawnRule> SpawnRules;
};
