// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/CAP_InteractInterface.h"
#include "CAP_WeaponBase.generated.h"

UCLASS()
class ACAP_WeaponBase : public AActor, public ICAP_InteractInterface
{
	GENERATED_BODY()
	
public:	
	ACAP_WeaponBase();
	virtual void BeginPlay() override;
	
	virtual void InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter) override;
	virtual void InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter) override;

	UPROPERTY(EditDefaultsOnly, Category="Weapon Data")
	class UCAP_WeaponDataAsset* WeaponDA;
	
protected:
	UPROPERTY(VisibleAnywhere, Category="Interaction")
	class USphereComponent* InteractionSphere;
	
	UFUNCTION()
	void OnInteractSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,const FHitResult& SweepResult);
	UFUNCTION()
	void OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
