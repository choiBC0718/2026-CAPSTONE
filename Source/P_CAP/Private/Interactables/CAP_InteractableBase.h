// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CAP_InteractInterface.h"
#include "CAP_InteractableBase.generated.h"

UCLASS()
class ACAP_InteractableBase : public AActor, public ICAP_InteractInterface
{
	GENERATED_BODY()
	
public:	

	ACAP_InteractableBase();

	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override {}
	virtual FInteractionPayload GetInteractionPayload() const override {return FInteractionPayload();}

	virtual void BeginPlay() override;

	void DropItem();
protected:
	UPROPERTY(VisibleAnywhere, Category="Interaction")
	class USphereComponent* RootCollision;
	// 플레이어 오버랩 상호작용 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="Interaction")
	class USphereComponent* InteractionSphere;
	
	UFUNCTION()
	void OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnRootCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
