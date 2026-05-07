#include "AnalysisObstacle.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "AI/PlayerTrackerComponent.h"

AAnalysisObstacle::AAnalysisObstacle()
{
	PrimaryActorTick.bCanEverTick = false;

	OuterZone = CreateDefaultSubobject<UBoxComponent>(TEXT("OuterZone"));
	RootComponent = OuterZone;
	OuterZone->SetCollisionProfileName(TEXT("Trigger"));

	InnerZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InnerZone"));
	InnerZone->SetupAttachment(RootComponent);
	InnerZone->SetCollisionProfileName(TEXT("Trigger"));

	bHasPassedThrough = false;
	bIsTracking = false;
}

void AAnalysisObstacle::BeginPlay()
{
	Super::BeginPlay();

	// [변경] 에디터에서 설정한 크기를 런타임에 적용
	OuterZone->SetBoxExtent(OuterZoneExtent);
	InnerZone->SetBoxExtent(InnerZoneExtent);

	OuterZone->OnComponentBeginOverlap.AddDynamic(this, &AAnalysisObstacle::OnOuterOverlapBegin);
	OuterZone->OnComponentEndOverlap.AddDynamic(this, &AAnalysisObstacle::OnOuterOverlapEnd);
	InnerZone->OnComponentBeginOverlap.AddDynamic(this, &AAnalysisObstacle::OnInnerOverlapBegin);
}

void AAnalysisObstacle::OnOuterOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		bIsTracking = true;
		bHasPassedThrough = false; 
	}
}

void AAnalysisObstacle::OnInnerOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsTracking && OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		bHasPassedThrough = true;
		
		UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
		if (Tracker)
		{
			Tracker->PassedObstacleCount++;
		}
		
		UE_LOG(LogTemp, Warning, TEXT("장애물 돌파(PassThrough) 감지"));
	}
}

void AAnalysisObstacle::OnOuterOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bIsTracking && OtherActor && OtherActor->IsA(ACharacter::StaticClass()))
	{
		if (!bHasPassedThrough)
		{
			UPlayerTrackerComponent* Tracker = OtherActor->FindComponentByClass<UPlayerTrackerComponent>();
			if (Tracker)
			{
				Tracker->AvoidedObstacleCount++;
			}
			UE_LOG(LogTemp, Warning, TEXT("장애물 회피(Avoid) 감지"));
		}
		
		bIsTracking = false;
		bHasPassedThrough = false;
	}
}