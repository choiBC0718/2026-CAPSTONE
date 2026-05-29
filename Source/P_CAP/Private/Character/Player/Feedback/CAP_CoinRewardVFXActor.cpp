#include "Character/Player/Feedback/CAP_CoinRewardVFXActor.h"

#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"

namespace CAPCoinRewardVFX
{
	const FName TargetLocationParam(TEXT("User.TargetLocation"));
	const FName BronzeCoinParam(TEXT("User.BronzeCoin"));
	const FName SilverCoinParam(TEXT("User.SilverCoin"));
	const FName GoldCoinParam(TEXT("User.GoldCoin"));
	const FName KillRadiusParam(TEXT("User.KillRad"));
	const FName SpeedParam(TEXT("User.Speed"));
	const FName FindRadiusParam(TEXT("User.FindRad"));
}

ACAP_CoinRewardVFXActor::ACAP_CoinRewardVFXActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(SceneRoot);
	NiagaraComponent->SetAutoActivate(false);
}

void ACAP_CoinRewardVFXActor::Play(const FCAPCoinRewardFeedbackParams& Params)
{
	FCAPCoinRewardFeedbackParams ResolvedParams = Params;
	if (!ResolvedParams.RewardVFX)
	{
		ResolvedParams.RewardVFX = DefaultRewardVFX;
	}

	if (bFeedbackStarted || !ResolvedParams.RewardVFX || !IsValid(ResolvedParams.TargetActor))
	{
		return;
	}

	bFeedbackStarted = true;
	FeedbackParams = ResolvedParams;
	FeedbackParams.AbsorbDelay = FMath::Max(0.35f, FeedbackParams.AbsorbDelay);
	AbsorbStartTime = GetWorld()->GetTimeSeconds() + FeedbackParams.AbsorbDelay;

	SetActorLocation(FeedbackParams.SourceLocation);
	SetLifeSpan(FMath::Max(5.f, FeedbackParams.AbsorbDelay + 5.f));

	const int32 GoldCoinCount = FMath::Max(0, FeedbackParams.CoinAmount);

	NiagaraComponent->SetAsset(FeedbackParams.RewardVFX);
	NiagaraComponent->SetVariableVec3(
		CAPCoinRewardVFX::TargetLocationParam,
		FeedbackParams.TargetActor->GetActorLocation());
	NiagaraComponent->SetVariableFloat(CAPCoinRewardVFX::KillRadiusParam, 0.f);
	NiagaraComponent->SetVariableFloat(CAPCoinRewardVFX::SpeedParam, 0.f);
	NiagaraComponent->SetVariableFloat(CAPCoinRewardVFX::FindRadiusParam, FeedbackParams.FindRadius);
	NiagaraComponent->SetVariableFloat(CAPCoinRewardVFX::GoldCoinParam, static_cast<float>(GoldCoinCount));
	NiagaraComponent->SetVariableFloat(CAPCoinRewardVFX::SilverCoinParam, 0.f);
	NiagaraComponent->SetVariableFloat(CAPCoinRewardVFX::BronzeCoinParam, 0.f);
	NiagaraComponent->OnSystemFinished.AddUniqueDynamic(this, &ACAP_CoinRewardVFXActor::HandleVFXFinished);
	NiagaraComponent->Activate(true);

	SetActorTickEnabled(true);
}

void ACAP_CoinRewardVFXActor::PlayAtLocation(
	AActor* TargetActor,
	FVector SourceLocation,
	int32 CoinAmount,
	float AbsorbDelay)
{
	FCAPCoinRewardFeedbackParams Params;
	Params.TargetActor = TargetActor;
	Params.SourceLocation = SourceLocation;
	Params.CoinAmount = CoinAmount;
	Params.AbsorbDelay = AbsorbDelay;
	Play(Params);
}

void ACAP_CoinRewardVFXActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsValid(FeedbackParams.TargetActor))
	{
		Destroy();
		return;
	}

	if (GetWorld()->GetTimeSeconds() >= AbsorbStartTime)
	{
		NiagaraComponent->SetVariableVec3(
			CAPCoinRewardVFX::TargetLocationParam,
			FeedbackParams.TargetActor->GetActorLocation());
		NiagaraComponent->SetVariableFloat(CAPCoinRewardVFX::KillRadiusParam, FeedbackParams.KillRadius);
		NiagaraComponent->SetVariableFloat(CAPCoinRewardVFX::SpeedParam, FeedbackParams.AbsorbSpeed);
	}
}

void ACAP_CoinRewardVFXActor::HandleVFXFinished(UNiagaraComponent* FinishedComponent)
{
	Destroy();
}
