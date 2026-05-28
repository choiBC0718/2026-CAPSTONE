#include "AutoPlayManager.h"
#include "BotPlayController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "AI/PlayerBehaviorLearner.h"
#include "AI/PlayerTrackerComponent.h"
#include "EngineUtils.h"

AAutoPlayManager::AAutoPlayManager()
{
	PrimaryActorTick.bCanEverTick = false;
	LastKnownHistoryCount = 0;
}

void AAutoPlayManager::BeginPlay()
{
	Super::BeginPlay();

	if (!bAutoPlayEnabled) return;

	CachedLearner = Cast<APlayerBehaviorLearner>(
		UGameplayStatics::GetActorOfClass(GetWorld(), APlayerBehaviorLearner::StaticClass()));

	if (!CachedLearner)
	{
		UE_LOG(LogTemp, Error, TEXT("AutoPlay: PlayerBehaviorLearner가 없습니다!"));
		return;
	}

	// 0.5초 대기 후 시작 — PlayerBehaviorLearner::BeginPlay()의 LoadLearnerData()가
	// 완료된 뒤 PlayHistory를 읽어야 완료 여부를 정확히 판단할 수 있음
	FTimerHandle StartTimer;
	GetWorld()->GetTimerManager().SetTimer(StartTimer, [this]()
	{
		int32 CompletedRuns = CachedLearner->PlayHistory.Num();
		LastKnownHistoryCount = CompletedRuns;

		if (CompletedRuns >= TotalAutoRuns)
		{
			// 학습 완료 → 속도 건드리지 않고 일반 플레이 모드
			UE_LOG(LogTemp, Warning, TEXT("자동 학습 완료 (%d/%d런)"), CompletedRuns, TotalAutoRuns);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green,
					FString::Printf(TEXT("자동 학습 완료! (%d런) - 직접 플레이 가능"), CompletedRuns));
			}
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("=== 자동 학습 런 %d/%d 시작 ==="), CompletedRuns + 1, TotalAutoRuns);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Magenta,
				FString::Printf(TEXT("=== 자동 학습 런 %d/%d ==="), CompletedRuns + 1, TotalAutoRuns));
		}

		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), GameSpeed);
		UE_LOG(LogTemp, Warning, TEXT("AutoPlay: 게임 속도 %.1f배"), GameSpeed);
		StartBotRun();

		GetWorld()->GetTimerManager().SetTimer(CompletionCheckTimer, this,
			&AAutoPlayManager::CheckRunCompletion, 1.0f, true);
	}, 0.5f, false);
}

void AAutoPlayManager::StartBotRun()
{
	// GetPlayerCharacter는 possession 타이밍에 따라 null일 수 있으므로
	// PlayerTrackerComponent로 직접 캐릭터를 탐색
	ACharacter* PlayerChar = nullptr;
	for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
	{
		if (It->FindComponentByClass<UPlayerTrackerComponent>())
		{
			PlayerChar = *It;
			break;
		}
	}

	if (!PlayerChar)
	{
		UE_LOG(LogTemp, Warning, TEXT("AutoPlay: 플레이어 캐릭터 아직 없음 → 1초 후 재시도"));
		FTimerHandle RetryTimer;
		GetWorld()->GetTimerManager().SetTimer(RetryTimer, [this]()
		{
			StartBotRun();
		}, 1.0f, false);
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC) PC->UnPossess();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ActiveBot = GetWorld()->SpawnActor<ABotPlayController>(
		ABotPlayController::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (ActiveBot)
	{
		// 프로필 지정 없음 → 봇이 OnPossess에서 알아서 랜덤 파라미터 생성
		ActiveBot->Possess(PlayerChar);
	}
}

void AAutoPlayManager::CheckRunCompletion()
{
	if (!CachedLearner) return;

	int32 CurrentCount = CachedLearner->PlayHistory.Num();

	if (CurrentCount > LastKnownHistoryCount)
	{
		LastKnownHistoryCount = CurrentCount;
		GetWorld()->GetTimerManager().ClearTimer(CompletionCheckTimer);

		if (CurrentCount >= TotalAutoRuns)
		{
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
			UE_LOG(LogTemp, Warning, TEXT("===== 자동 학습 완료! 총 %d런 (속도 원복) ====="), TotalAutoRuns);

			// 플레이어 컨트롤러를 캐릭터에 다시 연결
			ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
			APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			if (PC && PlayerChar && !PC->GetPawn())
			{
				PC->Possess(PlayerChar);
				UE_LOG(LogTemp, Warning, TEXT("플레이어 컨트롤러 복구 완료 → 직접 플레이 가능"));
			}

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green,
					FString::Printf(TEXT("자동 학습 완료! %d런 데이터 수집 → 직접 플레이 가능"), TotalAutoRuns));
			}
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("런 %d 완료 → 다음 런 준비"), CurrentCount);

		FTimerHandle ReloadTimer;
		GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this,
			&AAutoPlayManager::ReloadLevel, RestartDelay, false);
	}
}

void AAutoPlayManager::ReloadLevel()
{
	FString LevelName = GetWorld()->GetName();
	UGameplayStatics::OpenLevel(this, FName(*LevelName));
}