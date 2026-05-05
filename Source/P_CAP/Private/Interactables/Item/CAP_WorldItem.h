// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_ItemInstance.h"
#include "Interactables/CAP_InteractableBase.h"
#include "CAP_WorldItem.generated.h"

/**
 * 월드에 스폰되는 상호작용 가능한 아이템
 */
UCLASS()
class ACAP_WorldItem : public ACAP_InteractableBase
{
	GENERATED_BODY()

public:
	ACAP_WorldItem();
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void Interact(class AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;

	virtual void DropItem() override;
	
	UPROPERTY(EditDefaultsOnly, Category="Item Data")
	class UCAP_ItemDataAsset* ItemDA;

	UPROPERTY(BlueprintReadWrite, Category="Item Data", meta=(ExposeOnSpawn="true"))
	class UCAP_ItemInstance* ItemInstance;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USphereComponent* RootCollision;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class USceneComponent* MeshContainer;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class UStaticMeshComponent* ItemMesh;
	UPROPERTY(VisibleAnywhere, Category="Component")
	class URotatingMovementComponent* RotatingMovement;

	UFUNCTION()
	void OnRootCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
