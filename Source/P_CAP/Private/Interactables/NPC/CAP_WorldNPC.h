// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/CAP_DialogueComponent.h"
#include "Interactables/CAP_InteractableBase.h"
#include "CAP_WorldNPC.generated.h"


UENUM(BlueprintType)
enum class EEnhanceResult : uint8
{
	Default,
	Success,
	InsufficientCurrency,
	MaxGradeReached,
	ConfirmMode,
	Error,
};

/**
 * 
 */
UCLASS()
class ACAP_WorldNPC : public ACAP_InteractableBase
{
	GENERATED_BODY()

public:
	ACAP_WorldNPC();

	virtual void BeginPlay() override;
	
	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;

	UPROPERTY(EditAnywhere, Category="SpecialAction")
	TSubclassOf<class UUserWidget> NPCSpecialActionWidgetClass;

	virtual void ResetActionPending() {bIsActionPending = false;}
	
protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USkeletalMeshComponent* NPCMesh;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class UCAP_DialogueComponent* DialogueComponent;
	
	virtual ENPCActionResult ExecuteSpecialAction(AActor* Actor) {return ENPCActionResult::FirstInteraction;}
	bool GetSpecialActionCost(int& OutCost);
	
	UPROPERTY(EditAnywhere, Category="Drop")
	int32 CostMagicStone = 10;

	int32 InteractionCount = 0;

	bool bIsPaymentPending = false;
	bool bIsActionPending = false;
};
