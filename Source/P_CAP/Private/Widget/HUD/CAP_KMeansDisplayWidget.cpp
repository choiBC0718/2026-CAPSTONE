#include "Widget/HUD/CAP_KMeansDisplayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Map/Debug/MapManager.h"

void UCAP_KMeansDisplayWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshDisplay();
}

void UCAP_KMeansDisplayWidget::RefreshDisplay()
{
	AMapManager* MapManager = Cast<AMapManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMapManager::StaticClass()));

	if (MapManager)
	{
		ApplyTendency(MapManager->LastAppliedTendency);
	}
}

void UCAP_KMeansDisplayWidget::ApplyTendency(const FPlayerTendencyModifier& Tendency)
{
	if (PB_Combat)
	{
		PB_Combat->SetPercent(Tendency.CombatAggression);
		const float Scale = FMath::Lerp(0.5f, 1.5f, Tendency.CombatAggression);
		if (TB_CombatDesc)
			TB_CombatDesc->SetText(FText::FromString(
				FString::Printf(TEXT("전투 공격성  %.2f  →  몬스터 x%.1f배"), Tendency.CombatAggression, Scale)));
	}

	if (PB_Explore)
	{
		PB_Explore->SetPercent(Tendency.ExplorationRate);
		const FString Pattern = Tendency.ExplorationRate >= 0.5f ? TEXT("외곽 분산") : TEXT("통로 집중");
		if (TB_ExploreDesc)
			TB_ExploreDesc->SetText(FText::FromString(
				FString::Printf(TEXT("탐험 성향    %.2f  →  %s"), Tendency.ExplorationRate, *Pattern)));
	}

	if (PB_Obstacle)
	{
		PB_Obstacle->SetPercent(Tendency.ObstacleBypass);
		const int32 Count = FMath::RoundToInt(2.f * Tendency.ObstacleBypass);
		if (TB_ObstacleDesc)
			TB_ObstacleDesc->SetText(FText::FromString(
				FString::Printf(TEXT("장애물 돌파  %.2f  →  %d개/방"), Tendency.ObstacleBypass, Count)));
	}
}
