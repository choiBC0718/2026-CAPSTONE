// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CAP_AIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

ACAP_AIController::ACAP_AIController()
{
}

void ACAP_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	SetAIEnabled(true);
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

	if (!bEnabled)
	{
		StopMovement();
		CachedTargetActor = nullptr;
	}
	else if (!CachedTargetActor)
	{
		CachedTargetActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	ApplyBlackboardValues();
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

void ACAP_AIController::SetCurrentTarget(AActor* NewTarget)
{
	CachedTargetActor = NewTarget;
	ApplyBlackboardValues();
}
