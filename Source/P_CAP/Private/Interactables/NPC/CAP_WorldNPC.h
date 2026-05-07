// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/CAP_InteractableBase.h"
#include "CAP_WorldNPC.generated.h"


USTRUCT(BlueprintType)
struct FNPCData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FString NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	class UTexture2D* NPCImage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(MultiLine=true))
	FString DefaultDialogue;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FString SpecialActionText;
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

	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USkeletalMeshComponent* NPCMesh;

	UPROPERTY(EditAnywhere, Category="Dialogue")
	FNPCData NPCData;
};
