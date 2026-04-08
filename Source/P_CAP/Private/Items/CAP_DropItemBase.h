// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CAP_InteractInterface.h"
#include "CAP_DropItemBase.generated.h"

UCLASS()
class ACAP_DropItemBase : public AActor, public ICAP_InteractInterface
{
	GENERATED_BODY()
	
public:	

	ACAP_DropItemBase();

	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter) override;
	virtual void InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter) override;

	void DropItem();
protected:
	// 땅과 충돌할 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="Interaction")
	class USphereComponent* RootCollision;
	// 플레이어 오버랩 상호작용 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="Interaction")
	class USphereComponent* InteractionSphere;
	// 아이템 메시의 부모
	UPROPERTY()
	class USceneComponent* MeshContainer;
	// 액터 회전 컴포넌트
	UPROPERTY(VisibleAnywhere, Category="Component")
	class URotatingMovementComponent* RotatingMovementComp;

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	float BobbingSpeed = 3.f;
	UPROPERTY(EditDefaultsOnly, Category="Animation")
	float BobbingHeight = 5.f;

	UFUNCTION()
	void OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	UFUNCTION()
	void OnDropCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
