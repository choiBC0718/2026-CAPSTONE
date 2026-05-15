// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CAP_AIController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

ACAP_AIController::ACAP_AIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->SightRadius = 1200.f;
	SightConfig->LoseSightRadius = 1500.f;
	SightConfig->PeripheralVisionAngleDegrees = 90.f;
	SightConfig->SetMaxAge(1.f);

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ACAP_AIController::TargetPerceptionUpdated);
	AIPerceptionComp->OnTargetPerceptionForgotten.AddDynamic(this, &ACAP_AIController::TargetForgotten);
}

void ACAP_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ClearAndDisableAllSenses();
	SetAIEnabled(false);
}

void ACAP_AIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviorTree);
	ApplyBlackboardValues();
}

void ACAP_AIController::SetAIEnabled(bool bEnabled)
{
	bAIEnabled = bEnabled;

	if (bEnabled)
	{
		EnableAllSenses();
	}
	else
	{
		StopMovement();
		ClearAndDisableAllSenses();
	}

	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->SetValueAsBool(AIEnabledBlackboardKeyName, bAIEnabled);
	}
}

void ACAP_AIController::SetTargetActor(AActor* TargetActor)
{
	SetCurrentTarget(TargetActor);
}

void ACAP_AIController::ApplyBlackboardValues()
{
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->SetValueAsBool(AIEnabledBlackboardKeyName, bAIEnabled);

		if (CachedTargetActor)
		{
			BlackboardComponent->SetValueAsObject(TargetBlackboardKeyName, CachedTargetActor);
		}
		else
		{
			BlackboardComponent->ClearValue(TargetBlackboardKeyName);
		}
	}
}

void ACAP_AIController::TargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (!bAIEnabled || !TargetActor)
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		if (!GetCurrentTarget())
		{
			SetCurrentTarget(TargetActor);
		}
	}
	else
	{
		ForgetActorIfDead(TargetActor);
	}
}

void ACAP_AIController::TargetForgotten(AActor* ForgottenActor)
{
	if (!bAIEnabled || !ForgottenActor)
	{
		return;
	}

	if (GetCurrentTarget() == ForgottenActor)
	{
		SetCurrentTarget(GetNextPerceivedActor());
	}
}

const UObject* ACAP_AIController::GetCurrentTarget() const
{
	const UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	return BlackboardComponent ? BlackboardComponent->GetValueAsObject(TargetBlackboardKeyName) : nullptr;
}

void ACAP_AIController::SetCurrentTarget(AActor* NewTarget)
{
	CachedTargetActor = NewTarget;
	ApplyBlackboardValues();
}

AActor* ACAP_AIController::GetNextPerceivedActor() const
{
	if (!AIPerceptionComp || !SightConfig)
	{
		return nullptr;
	}

	TArray<AActor*> Actors;
	AIPerceptionComp->GetCurrentlyPerceivedActors(SightConfig->GetSenseImplementation(), Actors);

	for (AActor* Actor : Actors)
	{
		if (Actor && Actor != GetPawn())
		{
			return Actor;
		}
	}

	return nullptr;
}

void ACAP_AIController::ForgetActorIfDead(AActor* ActorToForget)
{
	const UAbilitySystemComponent* ActorASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ActorToForget);
	if (!ActorASC)
	{
		return;
	}

	if (ActorASC->HasMatchingGameplayTag(UCAP_AbilitySystemStatics::GetDeadStateTag()) &&
		GetCurrentTarget() == ActorToForget)
	{
		SetCurrentTarget(GetNextPerceivedActor());
	}
}

void ACAP_AIController::ClearAndDisableAllSenses()
{
	AIPerceptionComp->AgeStimuli(TNumericLimits<float>::Max());
	for (auto SenseConfigIt = AIPerceptionComp->GetSensesConfigIterator() ; SenseConfigIt ; ++SenseConfigIt)
	{
		AIPerceptionComp->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), false);
	}

	if (GetBlackboardComponent())
	{
		GetBlackboardComponent()->ClearValue(TargetBlackboardKeyName);
	}
}

void ACAP_AIController::EnableAllSenses()
{
	AIPerceptionComp->AgeStimuli(TNumericLimits<float>::Max());
	for (auto SenseConfigIt = AIPerceptionComp->GetSensesConfigIterator() ; SenseConfigIt ; ++SenseConfigIt)
	{
		AIPerceptionComp->SetSenseEnabled((*SenseConfigIt)->GetSenseImplementation(), true);
	}
}
