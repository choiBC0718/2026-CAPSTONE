// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Actors/CAP_SummonActor.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


ACAP_SummonActor::ACAP_SummonActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACAP_SummonActor::BeginPlay()
{
	Super::BeginPlay();
	TargetPlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

// Called every frame
void ACAP_SummonActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TargetPlayer)
	{
		FVector CurrentLoc = GetActorLocation();
		FVector TargetLoc = TargetPlayer->GetActorLocation();

		FVector DirectionToSummon = (CurrentLoc - TargetLoc).GetSafeNormal();
		if (DirectionToSummon.IsNearlyZero())
			DirectionToSummon = -TargetPlayer->GetActorForwardVector();

		FVector DesiredLoc = TargetLoc + (DirectionToSummon * FollowDistance);
		DesiredLoc.Z = TargetLoc.Z + 50.f;

		FVector NewLoc = FMath::VInterpTo(CurrentLoc, DesiredLoc, DeltaTime, FollowSpeed);
		FRotator NewRot = (TargetLoc - CurrentLoc).Rotation();
		NewRot.Pitch = 0.f;
		NewRot.Roll = 0.f;

		SetActorLocationAndRotation(NewLoc, NewRot);
	}
}

