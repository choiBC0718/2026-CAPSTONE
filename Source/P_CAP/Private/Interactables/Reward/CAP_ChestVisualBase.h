// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAP_ChestVisualBase.generated.h"

UCLASS()
class ACAP_ChestVisualBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ACAP_ChestVisualBase();

	virtual void BeginPlay() override;
	void PlayOpenAnim();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* BaseRoot;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
	class UNiagaraSystem* FlashEffect;
};
