#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AI/PlayerBehaviorLearner.h"
#include "CAP_KMeansDisplayWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class UCAP_KMeansDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="K-Means Display")
	void RefreshDisplay();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* PB_Combat;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* PB_Explore;

	UPROPERTY(meta=(BindWidget))
	UProgressBar* PB_Obstacle;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* TB_CombatDesc;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* TB_ExploreDesc;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* TB_ObstacleDesc;

private:
	void ApplyTendency(const FPlayerTendencyModifier& Tendency);
};
