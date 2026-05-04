// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Components/ActorComponent.h"
#include "CAP_InteractionComponent.generated.h"

// 오버랩 대상 변경됨 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, AActor*, InteractableActor);
// 아이템 분해 시 게이지 증가 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractProgressUpdated, float, Progress);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCAP_InteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCAP_InteractionComponent();
	
	AActor* GetNearbyInteractable() const {return NearbyInteractable;}
	void SetNearbyInteractable(AActor* NewActor);
	void ProcessInteractInput(ETriggerEvent TriggerEvent, float ElapsedTime);

	UPROPERTY()
	FOnInteractableChanged OnInteractableChanged;
	UPROPERTY()
	FOnInteractProgressUpdated OnInteractProgressUpdated;
	
private:
	UPROPERTY()
	AActor* NearbyInteractable;

	// 아이템 장착 허용 시간
	UPROPERTY(EditDefaultsOnly, Category="Input")
	float TapThreshold = 0.2f;

	// 아이템 분해 완료 시간
	UPROPERTY(EditDefaultsOnly, Category="Input")
	float HoldThreshold = 1.0f;
};
