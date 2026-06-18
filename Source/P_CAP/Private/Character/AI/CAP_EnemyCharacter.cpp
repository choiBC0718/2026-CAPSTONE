// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CAP_EnemyCharacter.h"

#include "Character/AI/CAP_AIController.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_CurrencyComponent.h"
#include "Character/Player/Feedback/CAP_CoinRewardVFXActor.h"
#include "Components/WidgetComponent.h"
#include "Data/CAP_MonsterRewardDataAsset.h"
#include "Framework/CAP_RewardSettings.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Widget/Common/CAP_OverheadStatsGauge.h"
#include "AI/PlayerTrackerComponent.h"
#include "AI/PlayerBehaviorLearner.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/Common/CAP_TargetEffectWidget.h"

namespace
{
int32 RollRewardAmount(const FIntPoint& AmountRange)
{
	const int32 MinAmount = FMath::Max(0, FMath::Min(AmountRange.X, AmountRange.Y));
	const int32 MaxAmount = FMath::Max(0, FMath::Max(AmountRange.X, AmountRange.Y));
	return MaxAmount > MinAmount ? FMath::RandRange(MinAmount, MaxAmount) : MinAmount;
}
}

ACAP_EnemyCharacter::ACAP_EnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickEnabled(false);

	bCanRespawn = false;

	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Health Bar Widget Component"));
	HealthBarWidgetComponent->SetupAttachment(GetRootComponent());
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComponent->SetDrawSize(FVector2D(120.f, 16.f));
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> CoinDropVFXFinder(
		TEXT("/Game/_Workspace/7_Monster/Reward/CoinDrop/NS_CoinDrop.NS_CoinDrop"));
	if (CoinDropVFXFinder.Succeeded())
	{
		CoinRewardVFX = CoinDropVFXFinder.Object;
	}
}

void ACAP_EnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this,this);
		ASC->InitComponent(CharacterStatRowName);
	}

	InitializeHealthBarWidget();
}

void ACAP_EnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bPlayingDeathDissolve)
	{
		SetActorTickEnabled(false);
		return;
	}

	DeathDissolveElapsedTime += DeltaSeconds;
	const float Alpha = DeathDissolveDuration > 0.f
		? FMath::Clamp(DeathDissolveElapsedTime / DeathDissolveDuration, 0.f, 1.f)
		: 1.f;
	const float DissolveValue = FMath::Lerp(DeathDissolveStartValue, DeathDissolveEndValue, Alpha);

	for (UMaterialInstanceDynamic* DynamicMaterial : DeathDissolveDynamicMaterials)
	{
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(DeathDissolveParameterName, DissolveValue);
		}
	}

	if (Alpha >= 1.f)
	{
		FinishDeathDissolve();
	}
}

void ACAP_EnemyCharacter::SetEnemyAIEnabled(bool bEnabled, AActor* TargetActor)
{
	bEnemyAIEnabled = bEnabled;
	CurrentTargetActor = bEnabled ? TargetActor : nullptr;

	ACAP_AIController* CAPAIController = Cast<ACAP_AIController>(GetController());
	if (!CAPAIController)
	{
		return;
	}

	CAPAIController->SetAIEnabled(bEnabled);
	CAPAIController->SetTargetActor(CurrentTargetActor);
}

void ACAP_EnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (bEnemyAIEnabled)
	{
		SetEnemyAIEnabled(true, CurrentTargetActor);
	}
}

void ACAP_EnemyCharacter::OnRoomActivated_Implementation(AActor* TargetActor)
{
	SetEnemyAIEnabled(true, TargetActor);
}

void ACAP_EnemyCharacter::OnRoomDeactivated_Implementation()
{
	SetEnemyAIEnabled(false);
}

void ACAP_EnemyCharacter::UpdateStackUI(const FGameplayTag& BehaviorTag, int32 CurrentStack, int32 MaxStack)
{
	if (TargetEffectWidgetComp)
	{
		if (UCAP_TargetEffectWidget* EffectWidget = Cast<UCAP_TargetEffectWidget>(TargetEffectWidgetComp->GetUserWidgetObject()))
			EffectWidget->UpdateEffectUI(BehaviorTag, CurrentStack,MaxStack);
	}
}

void ACAP_EnemyCharacter::OnDead()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UCAP_ProgressionSubsystem* ProgressionSub = GI->GetSubsystem<UCAP_ProgressionSubsystem>())
		{
			ProgressionSub->AddEnemyDefeated();
		}
	}
	
	SetEnemyAIEnabled(false);
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetVisibility(false);
	}
	GiveDeathCurrencyReward();
	SpawnCoinRewardVFX();

	// AITESTMAP처럼 PlayerBehaviorLearner가 있는 맵에서만 카운트
	bool bLearnerExists = false;
	for (TActorIterator<APlayerBehaviorLearner> It(GetWorld()); It; ++It)
	{
		bLearnerExists = true;
		break;
	}
	if (!bLearnerExists) return;

	// 순수 플레이어(APlayerController)만 카운트 — 봇(ABotPlayController)은 KillMonster()에서 직접 처리
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		UPlayerTrackerComponent* Tracker = It->FindComponentByClass<UPlayerTrackerComponent>();
		if (!Tracker) continue;

		if (!Cast<APlayerController>(It->GetController())) continue;

		float Dist = FVector::Dist(GetActorLocation(), It->GetActorLocation());
		if (Dist <= 300.f)
			Tracker->MeleeKillCount++;
		else
			Tracker->RangedKillCount++;

		Tracker->AddMonsterKill();
		break;
	}
}

void ACAP_EnemyCharacter::OnDeathMontageFinished()
{
	StartDeathDissolve();
}

void ACAP_EnemyCharacter::SpawnCoinRewardVFX()
{
	if (CoinRewardAmount <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	AActor* TargetActor = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(TargetActor))
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TSubclassOf<ACAP_CoinRewardVFXActor> VFXActorClass = CoinRewardVFXActorClass;
	if (!VFXActorClass)
	{
		VFXActorClass = ACAP_CoinRewardVFXActor::StaticClass();
	}

	const FVector CoinSpawnLocation = GetActorLocation() + CoinRewardSpawnOffset;

	ACAP_CoinRewardVFXActor* VFXActor = World->SpawnActor<ACAP_CoinRewardVFXActor>(
		VFXActorClass,
		CoinSpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams);

	if (!VFXActor)
	{
		return;
	}

	FCAPCoinRewardFeedbackParams Params;
	Params.RewardVFX = CoinRewardVFX;
	Params.TargetActor = TargetActor;
	Params.SourceLocation = CoinSpawnLocation;
	Params.CoinAmount = CoinRewardAmount;
	Params.AbsorbDelay = CoinRewardAbsorbDelay;
	Params.KillRadius = CoinRewardKillRadius;
	Params.AbsorbSpeed = CoinRewardAbsorbSpeed;
	Params.FindRadius = CoinRewardFindRadius;
	VFXActor->Play(Params);
}

void ACAP_EnemyCharacter::GiveDeathCurrencyReward()
{
	if (bDeathCurrencyRewardGranted)
	{
		return;
	}

	bDeathCurrencyRewardGranted = true;

	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("Death currency reward skipped: player pawn is missing for %s."), *GetName());
		return;
	}

	UCAP_CurrencyComponent* CurrencyComponent = Player->GetCurrencyComponent();
	if (!CurrencyComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Death currency reward skipped: currency component is missing for %s."), *GetName());
		return;
	}

	const UCAP_RewardSettings* RewardSettings = GetDefault<UCAP_RewardSettings>();
	if (!RewardSettings || RewardSettings->MonsterRewardDataAsset.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("Death currency reward skipped: MonsterRewardDataAsset is not set for %s."), *GetName());
		return;
	}

	const UCAP_MonsterRewardDataAsset* RewardDataAsset = RewardSettings->MonsterRewardDataAsset.LoadSynchronous();
	if (!RewardDataAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("Death currency reward skipped: MonsterRewardDataAsset failed to load for %s."), *GetName());
		return;
	}

	FCAPMonsterCurrencyReward ResolvedReward;
	if (!RewardDataAsset->FindRewardForMonster(GetClass(), RewardGroup, ResolvedReward))
	{
		UE_LOG(LogTemp, Warning, TEXT("Death currency reward skipped: reward data was not found for %s."), *GetName());
		return;
	}

	const int32 GoldRewardAmount = RollRewardAmount(ResolvedReward.GoldRewardAmountRange);
	if (GoldRewardAmount > 0)
	{
		CurrencyComponent->AddCurrency(ECurrencyType::Gold, GoldRewardAmount);
	}

	if (ResolvedReward.bCanDropMagicStone && ResolvedReward.MagicStoneDropChance > 0.f)
	{
		const float ClampedDropChance = FMath::Clamp(ResolvedReward.MagicStoneDropChance, 0.f, 1.f);
		if (FMath::FRand() <= ClampedDropChance)
		{
			const int32 MagicStoneRewardAmount = RollRewardAmount(ResolvedReward.MagicStoneRewardAmountRange);
			if (MagicStoneRewardAmount > 0)
			{
				CurrencyComponent->AddCurrency(ECurrencyType::MagicStone, MagicStoneRewardAmount);
			}
		}
	}
}

void ACAP_EnemyCharacter::StartDeathDissolve()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (!MeshComponent || !DeathDissolveMaterial || DeathDissolveDuration <= 0.f)
	{
		return;
	}
	DeathDissolveDynamicMaterials.Empty();

	const int32 MaterialCount = MeshComponent->GetNumMaterials();
	DeathDissolveDynamicMaterials.Reserve(MaterialCount);

	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateDynamicMaterialInstance(
			MaterialIndex,
			DeathDissolveMaterial);
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(DeathDissolveParameterName, DeathDissolveStartValue);
		}
		DeathDissolveDynamicMaterials.Add(DynamicMaterial);
	}

	bPlayingDeathDissolve = true;
	DeathDissolveElapsedTime = 0.f;
	SetActorTickEnabled(true);
}

void ACAP_EnemyCharacter::FinishDeathDissolve()
{
	bPlayingDeathDissolve = false;

	for (UMaterialInstanceDynamic* DynamicMaterial : DeathDissolveDynamicMaterials)
	{
		if (DynamicMaterial)
		{
			DynamicMaterial->SetScalarParameterValue(DeathDissolveParameterName, DeathDissolveEndValue);
		}
	}

	DeathDissolveDynamicMaterials.Empty();
	SetActorTickEnabled(false);
	Destroy();
}


void ACAP_EnemyCharacter::InitializeHealthBarWidget()
{
	if (!HealthBarWidgetComponent)
	{
		return;
	}

	if (HealthBarWidgetClass)
	{
		HealthBarWidgetComponent->SetWidgetClass(HealthBarWidgetClass);
	}

	UCAP_OverheadStatsGauge* HealthBarWidget = Cast<UCAP_OverheadStatsGauge>(HealthBarWidgetComponent->GetWidget());
	if (!HealthBarWidget)
	{
		return;
	}

	HealthBarWidget->ConfigureWithASC(GetAbilitySystemComponent());
}
