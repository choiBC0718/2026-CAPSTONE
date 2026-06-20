// Fill out your copyright notice in the Description page of Project Settings.

#include "Map/RoomActor/Monster/RoomMonsterSpawnerComponent.h"

#include "Character/AI/CAP_EnemyCharacter.h"
#include "Character/CAP_Character.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Map/RoomActor/Monster/RoomMonsterSpawnDataAsset.h"
#include "NiagaraFunctionLibrary.h"

namespace
{
	struct FRoomMonsterCellPlacement
	{
		FRoomMonsterSpawnPick SpawnPick;
		int32 CellIndex = INDEX_NONE;
		int32 MonsterIndexInCell = 0;
		int32 MonsterCountInCell = 1;
	};
}

URoomMonsterSpawnerComponent::URoomMonsterSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URoomMonsterSpawnerComponent::SetSpawnDataAsset(URoomMonsterSpawnDataAsset* InSpawnDataAsset)
{
	if (InSpawnDataAsset)
	{
		SpawnDataAsset = InSpawnDataAsset;
	}
}

void URoomMonsterSpawnerComponent::SpawnMonsters(
	const FRoomData& RoomData,
	const FRoomInteriorLayout& InteriorLayout,
	int32 MapSeed,
	const FTransform& RoomTransform,
	const FPlayerTendencyModifier& Tendency)
{
	const FRoomMonsterSpawnRule* SpawnRule = GetSpawnRule(RoomData.RoomType);
	if (!SpawnRule)
	{
		return;
	}

	SpawnMonstersFromRule(RoomData, InteriorLayout, MapSeed, RoomTransform, Tendency, *SpawnRule, 0, true);
}

int32 URoomMonsterSpawnerComponent::SpawnReinforcement(
	const FRoomData& RoomData,
	const FRoomInteriorLayout& InteriorLayout,
	int32 MapSeed,
	const FTransform& RoomTransform,
	const FPlayerTendencyModifier& Tendency,
	int32 ReinforcementIndex)
{
	const FRoomMonsterSpawnRule* SpawnRule = GetSpawnRule(RoomData.RoomType);
	if (!SpawnRule || !SpawnRule->Reinforcements.IsValidIndex(ReinforcementIndex))
	{
		return 0;
	}

	FRoomMonsterSpawnRule ReinforcementSpawnRule = *SpawnRule;
	const FRoomReinforcementRule& ReinforcementRule = SpawnRule->Reinforcements[ReinforcementIndex];
	ReinforcementSpawnRule.ScoreRange = ReinforcementRule.ScoreRange;
	return SpawnMonstersFromRule(RoomData, InteriorLayout, MapSeed, RoomTransform, Tendency, ReinforcementSpawnRule, ReinforcementIndex + 1, false, &ReinforcementRule);
}

int32 URoomMonsterSpawnerComponent::SpawnMonstersFromRule(
	const FRoomData& RoomData,
	const FRoomInteriorLayout& InteriorLayout,
	int32 MapSeed,
	const FTransform& RoomTransform,
	const FPlayerTendencyModifier& Tendency,
	const FRoomMonsterSpawnRule& SpawnRule,
	int32 RandomSalt,
	bool bClearExisting,
	const FRoomReinforcementRule* ReinforcementRule)
{
	if (bClearExisting)
	{
		ClearSpawnedMonsters();
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}

	// CombatAggression에 따라 ScoreRange 스케일 (0.5x ~ 1.5x)
	const float AggressionScale = FMath::Lerp(0.5f, 1.5f, Tendency.CombatAggression);

	FRoomMonsterSpawnRule AdjustedRule = SpawnRule;
	AdjustedRule.ScoreRange.X = FMath::Max(1, FMath::RoundToInt(SpawnRule.ScoreRange.X * AggressionScale));
	AdjustedRule.ScoreRange.Y = FMath::Max(1, FMath::RoundToInt(SpawnRule.ScoreRange.Y * AggressionScale));

	UE_LOG(LogTemp, Log, TEXT("[Spawn] Room(%d,%d) | Combat=%.2f AggrScale=%.2f | Score %d~%d → %d~%d"),
		RoomData.GridPos.X, RoomData.GridPos.Y,
		Tendency.CombatAggression, AggressionScale,
		SpawnRule.ScoreRange.X, SpawnRule.ScoreRange.Y,
		AdjustedRule.ScoreRange.X, AdjustedRule.ScoreRange.Y);

	FRandomStream RandomStream = MakeRoomRandomStream(RoomData, MapSeed, RandomSalt);
	TArray<FRoomMonsterSpawnPick> MonsterSpawnList = BuildMonsterSpawnList(AdjustedRule, RandomStream);
	if (MonsterSpawnList.IsEmpty())
	{
		return 0;
	}

	TArray<FIntPoint> SpawnableCells = CollectSpawnableCells(InteriorLayout);
	if (SpawnableCells.IsEmpty())
	{
		return 0;
	}

	// ExplorationRate=0 → 중앙 셀 우선, ExplorationRate=1 → 외곽 셀 우선
	SortCellsByCenterBias(SpawnableCells, InteriorLayout, RandomStream, Tendency.ExplorationRate);

	TArray<TMap<TSubclassOf<ACharacter>, int32>> CellMonsterCounts;
	CellMonsterCounts.SetNum(SpawnableCells.Num());

	TArray<TSubclassOf<ACharacter>> CellMonsterClasses;
	CellMonsterClasses.SetNum(SpawnableCells.Num());

	TArray<FRoomMonsterCellPlacement> Placements;
	Placements.Reserve(MonsterSpawnList.Num());

	for (const FRoomMonsterSpawnPick& SpawnPick : MonsterSpawnList)
	{
		for (int32 CellIndex = 0; CellIndex < SpawnableCells.Num(); ++CellIndex)
		{
			if (CellMonsterClasses[CellIndex] && CellMonsterClasses[CellIndex] != SpawnPick.MonsterClass)
			{
				continue;
			}

			const int32 CurrentCellCount = CellMonsterCounts[CellIndex].FindRef(SpawnPick.MonsterClass);
			if (CurrentCellCount >= FMath::Max(1, SpawnPick.MaxMonstersPerCell))
			{
				continue;
			}

			FRoomMonsterCellPlacement& Placement = Placements.AddDefaulted_GetRef();
			Placement.SpawnPick = SpawnPick;
			Placement.CellIndex = CellIndex;
			Placement.MonsterIndexInCell = CurrentCellCount;

			CellMonsterCounts[CellIndex].FindOrAdd(SpawnPick.MonsterClass)++;
			CellMonsterClasses[CellIndex] = SpawnPick.MonsterClass;
			break;
		}
	}

	TArray<int32> TotalMonsterCountsByCell;
	TotalMonsterCountsByCell.Init(0, SpawnableCells.Num());
	for (const FRoomMonsterCellPlacement& Placement : Placements)
	{
		if (TotalMonsterCountsByCell.IsValidIndex(Placement.CellIndex))
		{
			TotalMonsterCountsByCell[Placement.CellIndex]++;
		}
	}

	TArray<int32> SpawnedMonsterCountsByCell;
	SpawnedMonsterCountsByCell.Init(0, SpawnableCells.Num());

	int32 SpawnedCount = 0;
	for (FRoomMonsterCellPlacement& Placement : Placements)
	{
		if (!SpawnableCells.IsValidIndex(Placement.CellIndex))
		{
			continue;
		}

		Placement.MonsterIndexInCell = SpawnedMonsterCountsByCell[Placement.CellIndex]++;
		Placement.MonsterCountInCell = TotalMonsterCountsByCell[Placement.CellIndex];

		FVector WorldLocation = FVector::ZeroVector;
		if (!TryBuildSpawnLocation(
			Placement.SpawnPick.MonsterClass,
			InteriorLayout,
			SpawnableCells[Placement.CellIndex],
			Placement.MonsterIndexInCell,
			Placement.MonsterCountInCell,
			RoomTransform,
			RandomStream,
			WorldLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Spawn] Failed to find free monster spawn location in Room(%d,%d). Monster skipped."),
				RoomData.GridPos.X,
				RoomData.GridPos.Y);
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		ACharacter* SpawnedMonster = World->SpawnActor<ACharacter>(
			Placement.SpawnPick.MonsterClass,
			WorldLocation,
			FRotator(0.f, RandomStream.FRandRange(0.f, 360.f), 0.f),
			SpawnParams);

		if (SpawnedMonster)
		{
			SpawnedMonster->SpawnDefaultController();
			SpawnedMonsters.Add(SpawnedMonster);
			++SpawnedCount;

			if (ReinforcementRule)
			{
				const FVector EffectLocation = WorldLocation + ReinforcementRule->SpawnVFXOffset;
				if (ReinforcementRule->SpawnVFX)
				{
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(
						World,
						ReinforcementRule->SpawnVFX,
						EffectLocation,
						SpawnedMonster->GetActorRotation(),
						ReinforcementRule->SpawnVFXScale);
				}

				if (ReinforcementRule->SpawnSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						World,
						ReinforcementRule->SpawnSound,
						EffectLocation);
				}
			}
		}
	}

	return SpawnedCount;
}

void URoomMonsterSpawnerComponent::ClearSpawnedMonsters()
{
	for (ACharacter* Monster : SpawnedMonsters)
	{
		if (IsValid(Monster))
		{
			Monster->Destroy();
		}
	}

	SpawnedMonsters.Empty();
}

void URoomMonsterSpawnerComponent::ActivateSpawnedMonsters(AActor* TargetActor)
{
	for (ACharacter* Monster : SpawnedMonsters)
	{
		ACAP_EnemyCharacter* EnemyCharacter = Cast<ACAP_EnemyCharacter>(Monster);
		if (IsValid(EnemyCharacter))
		{
			EnemyCharacter->OnRoomActivated(TargetActor);
		}
	}
}

void URoomMonsterSpawnerComponent::DeactivateSpawnedMonsters()
{
	for (ACharacter* Monster : SpawnedMonsters)
	{
		ACAP_EnemyCharacter* EnemyCharacter = Cast<ACAP_EnemyCharacter>(Monster);
		if (IsValid(EnemyCharacter))
		{
			EnemyCharacter->OnRoomDeactivated();
		}
	}
}

bool URoomMonsterSpawnerComponent::HasSpawnedMonsters() const
{
	for (const ACharacter* Monster : SpawnedMonsters)
	{
		if (IsValid(Monster))
		{
			return true;
		}
	}

	return false;
}

bool URoomMonsterSpawnerComponent::AreAllSpawnedMonstersDefeated() const
{
	const bool bHadTrackedMonsters = !SpawnedMonsters.IsEmpty();

	for (const ACharacter* Monster : SpawnedMonsters)
	{
		if (!IsValid(Monster))
		{
			continue;
		}

		const ACAP_Character* CAPCharacter = Cast<ACAP_Character>(Monster);
		if (!CAPCharacter || !CAPCharacter->IsDead())
		{
			return false;
		}
	}

	return bHadTrackedMonsters;
}

int32 URoomMonsterSpawnerComponent::GetAliveSpawnedMonsterCount() const
{
	int32 AliveCount = 0;
	for (const ACharacter* Monster : SpawnedMonsters)
	{
		if (!IsValid(Monster))
		{
			continue;
		}

		const ACAP_Character* CAPCharacter = Cast<ACAP_Character>(Monster);
		if (!CAPCharacter || !CAPCharacter->IsDead())
		{
			++AliveCount;
		}
	}

	return AliveCount;
}

const FRoomMonsterSpawnRule* URoomMonsterSpawnerComponent::GetSpawnRule(ERoomType RoomType) const
{
	return SpawnDataAsset ? SpawnDataAsset->FindRule(RoomType) : nullptr;
}

TArray<FRoomMonsterSpawnPick> URoomMonsterSpawnerComponent::BuildMonsterSpawnList(
	const FRoomMonsterSpawnRule& SpawnRule,
	FRandomStream& RandomStream) const
{
	TArray<FRoomMonsterSpawnPick> MonsterSpawnList;

	const int32 MinScore = FMath::Max(0, FMath::Min(SpawnRule.ScoreRange.X, SpawnRule.ScoreRange.Y));
	const int32 MaxScore = FMath::Max(0, FMath::Max(SpawnRule.ScoreRange.X, SpawnRule.ScoreRange.Y));
	if (MaxScore <= 0 || SpawnRule.MonsterPool.IsEmpty())
	{
		return MonsterSpawnList;
	}

	int32 RemainingScore = RandomStream.RandRange(MinScore, MaxScore);
	TMap<TSubclassOf<ACharacter>, int32> SelectedCounts;

	while (RemainingScore > 0)
	{
		const FRoomMonsterSpawnEntry* PickedEntry = PickMonsterEntry(
			SpawnRule,
			SelectedCounts,
			RemainingScore,
			RandomStream);

		if (!PickedEntry || !PickedEntry->MonsterClass)
		{
			break;
		}

		FRoomMonsterSpawnPick& SpawnPick = MonsterSpawnList.AddDefaulted_GetRef();
		SpawnPick.MonsterClass = PickedEntry->MonsterClass;
		SpawnPick.MaxMonstersPerCell = PickedEntry->MaxMonstersPerCell;

		SelectedCounts.FindOrAdd(PickedEntry->MonsterClass)++;
		RemainingScore -= FMath::Max(1, PickedEntry->Cost);
	}

	return MonsterSpawnList;
}

const FRoomMonsterSpawnEntry* URoomMonsterSpawnerComponent::PickMonsterEntry(
	const FRoomMonsterSpawnRule& SpawnRule,
	const TMap<TSubclassOf<ACharacter>, int32>& SelectedCounts,
	int32 RemainingScore,
	FRandomStream& RandomStream) const
{
	int32 TotalWeight = 0;

	for (const FRoomMonsterSpawnEntry& Entry : SpawnRule.MonsterPool)
	{
		if (!Entry.MonsterClass || Entry.Cost > RemainingScore || Entry.Weight <= 0)
		{
			continue;
		}

		const int32 CurrentCount = SelectedCounts.FindRef(Entry.MonsterClass);
		if (CurrentCount >= Entry.MaxCount)
		{
			continue;
		}

		TotalWeight += Entry.Weight;
	}

	if (TotalWeight <= 0)
	{
		return nullptr;
	}

	const int32 RandomPick = RandomStream.RandRange(1, TotalWeight);
	int32 RunningWeight = 0;

	for (const FRoomMonsterSpawnEntry& Entry : SpawnRule.MonsterPool)
	{
		if (!Entry.MonsterClass || Entry.Cost > RemainingScore || Entry.Weight <= 0)
		{
			continue;
		}

		const int32 CurrentCount = SelectedCounts.FindRef(Entry.MonsterClass);
		if (CurrentCount >= Entry.MaxCount)
		{
			continue;
		}

		RunningWeight += Entry.Weight;
		if (RandomPick <= RunningWeight)
		{
			return &Entry;
		}
	}

	return nullptr;
}

TArray<FIntPoint> URoomMonsterSpawnerComponent::CollectSpawnableCells(
	const FRoomInteriorLayout& InteriorLayout) const
{
	TArray<FIntPoint> SpawnableCells;
	SpawnableCells.Reserve(InteriorLayout.Cells.Num());

	for (const FRoomInteriorCell& Cell : InteriorLayout.Cells)
	{
		if (Cell.Type == ERoomInteriorCellType::Empty &&
			!IsNearReservedDoorCell(InteriorLayout, Cell.Coord) &&
			!IsNearRoomEdge(InteriorLayout, Cell.Coord))
		{
			SpawnableCells.Add(Cell.Coord);
		}
	}

	return SpawnableCells;
}

bool URoomMonsterSpawnerComponent::IsNearReservedDoorCell(
	const FRoomInteriorLayout& InteriorLayout,
	const FIntPoint& CellCoord) const
{
	const int32 SafeRadius = FMath::Max(0, DoorExclusionRadiusInCells);
	if (SafeRadius <= 0)
	{
		return false;
	}

	for (const FRoomInteriorCell& Cell : InteriorLayout.Cells)
	{
		if (Cell.Type != ERoomInteriorCellType::ReservedDoor)
		{
			continue;
		}

		const int32 DistanceX = FMath::Abs(Cell.Coord.X - CellCoord.X);
		const int32 DistanceY = FMath::Abs(Cell.Coord.Y - CellCoord.Y);
		if (FMath::Max(DistanceX, DistanceY) <= SafeRadius)
		{
			return true;
		}
	}

	return false;
}

bool URoomMonsterSpawnerComponent::IsNearRoomEdge(
	const FRoomInteriorLayout& InteriorLayout,
	const FIntPoint& CellCoord) const
{
	const int32 SafeRadius = FMath::Max(0, WallExclusionRadiusInCells);
	if (SafeRadius <= 0)
	{
		return false;
	}

	return CellCoord.X < SafeRadius ||
		CellCoord.Y < SafeRadius ||
		CellCoord.X >= InteriorLayout.GridWidth - SafeRadius ||
		CellCoord.Y >= InteriorLayout.GridHeight - SafeRadius;
}

void URoomMonsterSpawnerComponent::SortCellsByCenterBias(
	TArray<FIntPoint>& SpawnableCells,
	const FRoomInteriorLayout& InteriorLayout,
	FRandomStream& RandomStream,
	float ExplorationRate) const
{
	if (SpawnableCells.Num() <= 1)
	{
		return;
	}

	const FVector2D GridCenter(
		(InteriorLayout.GridWidth - 1) * 0.5f,
		(InteriorLayout.GridHeight - 1) * 0.5f);
	const float MaxDistance = FVector2D(GridCenter).Size();
	const float SafeMaxDistance = FMath::Max(KINDA_SMALL_NUMBER, MaxDistance);
	const float Jitter = 0.1f;

	TMap<FIntPoint, float> CellScores;
	CellScores.Reserve(SpawnableCells.Num());
	for (const FIntPoint& CellCoord : SpawnableCells)
	{
		const FVector2D CellPoint(CellCoord.X, CellCoord.Y);
		const float NormalizedDistance = (CellPoint - GridCenter).Size() / SafeMaxDistance;
		// ExplorationRate=0 → 중앙(낮은 거리) 우선, ExplorationRate=1 → 외곽(높은 거리) 우선
		const float DirectionScore = FMath::Lerp(NormalizedDistance, 1.f - NormalizedDistance, ExplorationRate);
		CellScores.Add(CellCoord, DirectionScore + RandomStream.FRandRange(-Jitter, Jitter));
	}

	SpawnableCells.Sort(
		[&](const FIntPoint& A, const FIntPoint& B)
		{
			return CellScores.FindRef(A) < CellScores.FindRef(B);
		});
}

FVector URoomMonsterSpawnerComponent::GetCellLocalCenter(
	const FRoomInteriorLayout& InteriorLayout,
	const FIntPoint& CellCoord) const
{
	const float GridWorldSizeX = InteriorLayout.GridWidth * InteriorLayout.CellSize;
	const float GridWorldSizeY = InteriorLayout.GridHeight * InteriorLayout.CellSize;
	const FVector GridMin(-GridWorldSizeX * 0.5f, -GridWorldSizeY * 0.5f, 0.f);

	return FVector(
		GridMin.X + (CellCoord.X * InteriorLayout.CellSize) + (InteriorLayout.CellSize * 0.5f),
		GridMin.Y + (CellCoord.Y * InteriorLayout.CellSize) + (InteriorLayout.CellSize * 0.5f),
		0.f);
}

FVector URoomMonsterSpawnerComponent::GetFormationOffset(
	int32 MonsterIndexInCell,
	int32 MonsterCountInCell,
	float CellSize) const
{
	if (MonsterCountInCell <= 1)
	{
		return FVector::ZeroVector;
	}

	const float Radius = FMath::Clamp(CellSize * 0.22f, 35.f, 90.f);
	const float AngleStep = 360.f / MonsterCountInCell;
	const float AngleDegrees = AngleStep * MonsterIndexInCell;

	return FVector(
		FMath::Cos(FMath::DegreesToRadians(AngleDegrees)) * Radius,
		FMath::Sin(FMath::DegreesToRadians(AngleDegrees)) * Radius,
		0.f);
}

bool URoomMonsterSpawnerComponent::TryBuildSpawnLocation(
	TSubclassOf<ACharacter> MonsterClass,
	const FRoomInteriorLayout& InteriorLayout,
	const FIntPoint& CellCoord,
	int32 MonsterIndexInCell,
	int32 MonsterCountInCell,
	const FTransform& RoomTransform,
	FRandomStream& RandomStream,
	FVector& OutWorldLocation) const
{
	if (!MonsterClass)
	{
		return false;
	}

	const int32 RetryCount = FMath::Max(1, SpawnLocationMaxRetries);
	for (int32 RetryIndex = 0; RetryIndex < RetryCount; ++RetryIndex)
	{
		FVector LocalLocation = GetCellLocalCenter(InteriorLayout, CellCoord);
		LocalLocation += GetFormationOffset(MonsterIndexInCell, MonsterCountInCell, InteriorLayout.CellSize);
		LocalLocation.X += RandomStream.FRandRange(-SpawnJitter, SpawnJitter);
		LocalLocation.Y += RandomStream.FRandRange(-SpawnJitter, SpawnJitter);
		LocalLocation.Z += SpawnZOffset;

		const FVector WorldLocation = RoomTransform.TransformPosition(LocalLocation);
		if (IsSpawnLocationFree(MonsterClass, WorldLocation))
		{
			OutWorldLocation = WorldLocation;
			return true;
		}
	}

	return false;
}

bool URoomMonsterSpawnerComponent::IsSpawnLocationFree(TSubclassOf<ACharacter> MonsterClass, const FVector& WorldLocation) const
{
	const UWorld* World = GetWorld();
	const ACharacter* MonsterDefault = MonsterClass ? MonsterClass->GetDefaultObject<ACharacter>() : nullptr;
	const UCapsuleComponent* CapsuleComponent = MonsterDefault ? MonsterDefault->GetCapsuleComponent() : nullptr;
	if (!World || !CapsuleComponent)
	{
		return false;
	}

	const float CapsuleRadius = FMath::Max(1.f, CapsuleComponent->GetScaledCapsuleRadius() + SpawnCollisionPadding);
	const float CapsuleHalfHeight = FMath::Max(CapsuleRadius, CapsuleComponent->GetScaledCapsuleHalfHeight() + SpawnCollisionPadding);
	const FCollisionShape SpawnShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(RoomMonsterSpawnOverlap), false, GetOwner());
	QueryParams.bFindInitialOverlaps = true;

	return !World->OverlapBlockingTestByChannel(
		WorldLocation,
		FQuat::Identity,
		ECC_Pawn,
		SpawnShape,
		QueryParams);
}

FRandomStream URoomMonsterSpawnerComponent::MakeRoomRandomStream(
	const FRoomData& RoomData,
	int32 MapSeed,
	int32 RandomSalt) const
{
	int32 Seed = MapSeed;
	Seed = HashCombineFast(Seed, GetTypeHash(RoomData.GridPos));
	Seed = HashCombineFast(Seed, 0x5C2F913B);
	Seed = HashCombineFast(Seed, RandomSalt);
	return FRandomStream(Seed);
}
