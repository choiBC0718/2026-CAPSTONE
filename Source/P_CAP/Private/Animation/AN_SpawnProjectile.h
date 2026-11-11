// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_SpawnProjectile.generated.h"



/**
 * 
 */
UCLASS()
class UAN_SpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAN_SpawnProjectile();
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	
	UPROPERTY(EditAnywhere, Category="Event", meta=(Categories="Ability.Event"))
	FGameplayTag EventTag;
	
	UPROPERTY(EditAnywhere, Category="Event")
	FName SocketName = TEXT("Muzzle");
};
