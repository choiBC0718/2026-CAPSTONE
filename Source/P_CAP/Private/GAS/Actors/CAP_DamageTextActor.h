// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAP_DamageTextActor.generated.h"

UCLASS()
class ACAP_DamageTextActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ACAP_DamageTextActor();
	virtual void BeginPlay() override;

	void PlayDamageText(float Damage, bool bIsCritical, bool bIsPlayer);
	UFUNCTION()
	void ReturnToPool();

protected:
	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* DamageWidgetComp;
};
