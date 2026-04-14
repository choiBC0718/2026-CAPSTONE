// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CAP_AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UCAP_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	void UpdateWeaponAnimData(class UCAP_WeaponDataAsset* WeaponDA);

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const {return Speed;}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMoving() const {return Speed !=0;}
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsNotMoving() const {return Speed ==0;}
	
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetAimYaw() const {return AimYaw;}

protected:
	UPROPERTY(BlueprintReadOnly)
	class UAnimSequence* CurrentIdleAnim;
	UPROPERTY(BlueprintReadOnly)
	class UAnimSequence* CurrentJogStartAnim;
	UPROPERTY(BlueprintReadOnly)
	class UAnimSequence* CurrentJoggingAnim;
	UPROPERTY(BlueprintReadOnly)
	class UAnimSequence* CurrentJogEndAnim;
	
	UPROPERTY(BlueprintReadOnly)
	float CurrentJogEndStartTime = 0.f;
	
private:
	UPROPERTY()
	class ACharacter* OwnerCharacter;
	UPROPERTY()
	class UCharacterMovementComponent* OwnerMovementComp;

	FVector Velocity;
	float Speed;
	float AimYaw;
};
