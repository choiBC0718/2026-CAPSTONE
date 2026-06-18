// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Subsystem/CAP_DamageTextSubsystem.h"

#include "Framework/CAP_UISettings.h"
#include "GAS/Actors/CAP_DamageTextActor.h"

void UCAP_DamageTextSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	TSubclassOf<ACAP_DamageTextActor> ActorClass = GetDefault<UCAP_UISettings>()->DamageTextActorClass;
	if (!ActorClass)
		return;

	for (int i=0 ; i<50 ; i++)
	{
		ACAP_DamageTextActor* SpawnedActor = InWorld.SpawnActor<ACAP_DamageTextActor>(ActorClass);
		if (SpawnedActor)
		{
			SpawnedActor->SetActorHiddenInGame(true);
			TextActorPool.Add(SpawnedActor);
		}
	}
}

void UCAP_DamageTextSubsystem::ShowDamage(AActor* TargetActor, float Damage, bool bIsCritical, bool bIsPlayer)
{
	if (!TargetActor)
		return;
	ACAP_DamageTextActor* TextActor = GetActorFromPool();
	if (TextActor)
	{
		TextActor->SetActorLocation(TargetActor->GetActorLocation() + FVector(0,FMath::RandRange(-20.f,20.f),100.f));
		TextActor->PlayDamageText(Damage,bIsCritical,bIsPlayer);
	}
}

class ACAP_DamageTextActor* UCAP_DamageTextSubsystem::GetActorFromPool()
{
	for (ACAP_DamageTextActor* Actor : TextActorPool)
	{
		if (Actor && Actor->IsHidden())
		{
			return Actor;
		}
	}
	// 풀이 꽉 찼을 경우 확장 스폰 로직 추가 가능
	return nullptr;
}
