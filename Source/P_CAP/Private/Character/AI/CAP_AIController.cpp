// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CAP_AIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"
#include "Perception/AISense_Sight.h"

ACAP_AIController::ACAP_AIController()
{
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>("AI Perception Component");
	SightConfig = CreateDefaultSubobject<UAISense_Sight>("Sight Config");
}

void ACAP_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ClearAndDisableAllSenses();
	EnableAllSenses();
}

void ACAP_AIController::BeginPlay()
{
	Super::BeginPlay();
	RunBehaviorTree(BehaviorTree);
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
