// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Actors/CAP_DamageTextActor.h"

#include "Components/WidgetComponent.h"
#include "Widget/Common/CAP_DamageTextWidget.h"

// Sets default values
ACAP_DamageTextActor::ACAP_DamageTextActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	DamageWidgetComp=CreateDefaultSubobject<UWidgetComponent>("Damage Widget");
	RootComponent=DamageWidgetComp;
	DamageWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void ACAP_DamageTextActor::BeginPlay()
{
	Super::BeginPlay();
	if (UCAP_DamageTextWidget* DamageWidget = Cast<UCAP_DamageTextWidget>(DamageWidgetComp->GetUserWidgetObject()))
	{
		DamageWidget->OnDamageAnimFinished.AddDynamic(this, &ACAP_DamageTextActor::ReturnToPool);
	}
}

void ACAP_DamageTextActor::PlayDamageText(float Damage, bool bIsCritical, bool bIsPlayer)
{
	SetActorHiddenInGame(false);
	if (UCAP_DamageTextWidget* DamageWidget = Cast<UCAP_DamageTextWidget>(DamageWidgetComp->GetUserWidgetObject()))
	{
		if (!DamageWidget->OnDamageAnimFinished.IsAlreadyBound(this, &ACAP_DamageTextActor::ReturnToPool))
		{
			DamageWidget->OnDamageAnimFinished.AddDynamic(this, &ACAP_DamageTextActor::ReturnToPool);
		}
		DamageWidget->PlayDamageAnimation(Damage, bIsCritical, bIsPlayer);
	}
}

void ACAP_DamageTextActor::ReturnToPool()
{
	SetActorLocation(FVector(0.f, 0.f, -99999.f));
}

