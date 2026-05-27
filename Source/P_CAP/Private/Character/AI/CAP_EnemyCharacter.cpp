// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AI/CAP_EnemyCharacter.h"

#include "Character/AI/CAP_AIController.h"
#include "Components/WidgetComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Widget/Common/CAP_OverheadStatsGauge.h"
#include "AI/PlayerTrackerComponent.h"
#include "AI/PlayerBehaviorLearner.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Widget/Common/CAP_TargetEffectWidget.h"

ACAP_EnemyCharacter::ACAP_EnemyCharacter()
{
	bCanRespawn = false;

	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Health Bar Widget Component"));
	HealthBarWidgetComponent->SetupAttachment(GetRootComponent());
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComponent->SetDrawSize(FVector2D(120.f, 16.f));
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
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
	SetEnemyAIEnabled(false);

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

		if (!Cast<APlayerController>(It->GetController())) break;

		float Dist = FVector::Dist(GetActorLocation(), It->GetActorLocation());
		if (Dist <= 300.f)
			Tracker->MeleeKillCount++;
		else
			Tracker->RangedKillCount++;

		Tracker->AddMonsterKill();
		break;
	}
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
