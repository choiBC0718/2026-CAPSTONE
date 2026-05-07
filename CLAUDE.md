# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Unreal Engine 5.4 C++ game project (`P_CAP` module). The core research topic is **adaptive difficulty via player behavior learning**: a K-Means clustering pipeline reads spatial and combat data from real (or simulated) playthroughs, then adjusts monster spawning patterns accordingly.

## Build & Development Commands

Unreal Engine 5.4 must be installed and associated with the project.

**Generate Visual Studio project files:**
```powershell
# Run from repo root or via right-click on .uproject
"<UE5_ROOT>\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -project="C:\Users\Beom\Documents\2026-CAPSTONE\P_CAP.uproject" -game
```

**Compile (Development Editor):**
```powershell
msbuild P_CAP.sln /p:Configuration="Development Editor" /p:Platform=Win64
```

**Run / Play-in-Editor:** Open `P_CAP.uproject` in UE 5.4 and press Play. Use `AITESTMAP` for AI/learning tests and `TestMap` for general gameplay.

**Package build:**
```
UE5 Editor → Platforms → Windows → Package Project (Development or Shipping)
```

There is no separate lint or unit-test command — correctness is verified by compile + PIE (Play In Editor).

## Module & Dependencies

Defined in `Source/P_CAP/P_CAP.Build.cs`:

- **Public**: Core, CoreUObject, Engine, InputCore, EnhancedInput, Paper2D, Niagara, StructUtils, NavigationSystem
- **Private**: GameplayAbilities, GameplayTasks, GameplayTags, UMG, Slate, SlateCore
- **Plugins**: GameplayAbilities (GAS), ApexDestruction

## Architecture

### Player Behavior Learning Pipeline

The central system. Data flows:

1. **`UPlayerTrackerComponent`** (attached to player) — records location periodically to the quadtree, counts kills (`KilledMonsterCount`, `MeleeKillCount`, `RangedKillCount`), and tracks obstacle events (`PassedObstacleCount`, `AvoidedObstacleCount`).
2. **`UQuadtreeManager`** — spatial partitioner that divides the map into a 4-tree. Stores visit frequency per node (`FQuadtreeNode`). Used to derive the player's exploration breadth.
3. **`UPlayerBehaviorLearner`** — aggregates run data into `FPlayerBehaviorData` structs and runs K-Means clustering. Outputs a `FPlayerTendencyModifier` with four normalized (0–1) dimensions: `ExplorationRate`, `CombatAggression`, `MeleePreference`, `ObstacleBypass`. Results persist via `ULearnerSaveGame` and can be exported to CSV.
4. **`AMonsterDirector`** — reads the tendency and spawns monsters accordingly (SpeedRunner playstyle → center-of-map spawns; Explorer playstyle → edge spawns). Owns a `UBoxComponent` spawn volume.

### Automated Playthrough Loop (Bot Testing)

`AAutoPlayManager` drives N automated sessions (default 20) at up to 10× game speed. Each session uses `ABotPlayController`, which simulates a player with a finite-state machine: `Idle → Roaming → ApproachingMonster → AvoidingObstacle → PassingObstacle → Finished`. Bot behavior is parameterized by `CombatWeight`, `MeleePreference`, and `ObstaclePassPreference`, allowing varied training data for the learner.

### Character & GAS Hierarchy

```
ACharacter
└── ACAP_Character  (IAbilitySystemInterface — holds AbilitySystemComponent + AttributeSet)
    ├── ACAP_PlayerCharacter  (EnhancedInput, weapon slots, UPlayerTrackerComponent, interact UI)
    └── ACAP_EnemyCharacter

AAIController
├── ACAP_AIController   (BehaviorTree + UAISense_Sight, blackboard key "Target")
└── ABotPlayController  (FSM-based bot simulation, no BT)
ABaseMonster            (inherits ACharacter; dual attack zones: InnerAttackZone ~200u, OuterAttackZone ~800u)
```

Abilities are triggered via `AbilityInputActions` map on `ACAP_PlayerCharacter`; weapons are data-asset driven (`UCAP_WeaponDataAsset`), swappable via `SwapWeapon()`.

### Obstacle Analytics

`AAnalysisObstacle` places two collision volumes (`OuterZone`, `InnerZone`) around each obstacle. When the player enters `OuterZone` but not `InnerZone`, it counts as avoidance; passing through both counts as a pass-through. These events feed `UPlayerTrackerComponent`.

`AStageGoalTrigger` marks the exit/goal zone and stores normalization constants (`MaxExpectedPlayTime` = 120 s, `ExpectedMaxVisitRatio` = 0.4) used by the learner to scale raw metrics.

### Map & Framework

Map generation logic lives in `Source/P_CAP/Private/Map/` (RoomActor, MapDebugActor split). Game mode and session setup are in `Framework/`. UI (HUD, common widgets) is in `Widget/`.

## Key Data Structures

| Struct | Location | Purpose |
|--------|----------|---------|
| `FPlayerTendencyModifier` | PlayerBehaviorLearner.h | Output of K-Means: 4 float dimensions describing playstyle |
| `FPlayerBehaviorData` | PlayerBehaviorLearner.h | Per-run input data; has `Lerp()` and `Distance()` helpers |
| `FQuadtreeNode` | QuadtreeManager.h | Tree node with `Center`, `Extent`, `NodeID`, `VisitCount`, `Children[4]` |

## Important Configuration

`Config/DefaultEngine.ini` sets:
- Default maps: `TestMap` (gameplay) and `AITESTMAP` (AI testing)
- Renderer: DX12 / SM6, dynamic GI, virtual shadow maps, mesh distance fields
- Audio: 48 kHz

Cold-start default for all tendency values is **0.5** (neutral) when insufficient history exists.
