#include "BaseMonster.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "AI/PlayerTrackerComponent.h"
#include "EngineUtils.h"

ABaseMonster::ABaseMonster()
{
	PrimaryActorTick.bCanEverTick = false;

	// 컴포넌트 생성 (기본 크기로 초기화, BeginPlay에서 프로퍼티 값 적용)
	OuterAttackZone = CreateDefaultSubobject<USphereComponent>(TEXT("OuterAttackZone"));
	OuterAttackZone->SetupAttachment(RootComponent);
	OuterAttackZone->SetCollisionProfileName(TEXT("Trigger"));

	InnerAttackZone = CreateDefaultSubobject<USphereComponent>(TEXT("InnerAttackZone"));
	InnerAttackZone->SetupAttachment(RootComponent);
	InnerAttackZone->SetCollisionProfileName(TEXT("Trigger"));
}

void ABaseMonster::BeginPlay()
{
	Super::BeginPlay();

	// [변경] 에디터에서 설정한 값을 런타임에 적용
	OuterAttackZone->SetSphereRadius(OuterAttackRadius);
	InnerAttackZone->SetSphereRadius(InnerAttackRadius);
}

void ABaseMonster::Die()
{
	// GetPlayerCharacter는 봇 모드에서 null → 폰 직접 탐색
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		UPlayerTrackerComponent* Tracker = It->FindComponentByClass<UPlayerTrackerComponent>();
		if (Tracker)
		{
			Tracker->AddMonsterKill();
			break;
		}
	}
	Destroy();
}

void ABaseMonster::ReceiveAttack(AActor* Attacker)
{
	if (!Attacker) return;

	UPlayerTrackerComponent* Tracker = Attacker->FindComponentByClass<UPlayerTrackerComponent>();

	if (InnerAttackZone->IsOverlappingActor(Attacker))
	{
		UE_LOG(LogTemp, Warning, TEXT("판정: 근접(Melee) 처치!"));
		if (Tracker) Tracker->MeleeKillCount++;
	}
	else if (OuterAttackZone->IsOverlappingActor(Attacker))
	{
		UE_LOG(LogTemp, Warning, TEXT("판정: 원거리(Ranged) 처치!"));
		if (Tracker) Tracker->RangedKillCount++;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("판정: 초장거리 처치 (Outer 구역 밖)"));
	}

	Die();
}