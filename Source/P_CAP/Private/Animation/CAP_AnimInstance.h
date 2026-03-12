// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SceneRenderTargetParameters.h"
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

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const {return Speed;}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMoving() const {return Speed !=0; }
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsNotMoving() const {return Speed ==0; }
	
	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetVerticalInput() const
	{
		if (Velocity.IsNearlyZero())
			return 0.f;
		const FVector Forward = BodyPrevRot.Vector();
		return FVector::DotProduct(Velocity, Forward);
	}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetHorizontalInput() const
	{
		if (Velocity.IsNearlyZero())
			return 0.f;
		const FVector Right = FRotationMatrix(BodyPrevRot).GetScaledAxis(EAxis::Y);
		return FVector::DotProduct(Velocity, Right);
	}
	
private:
	UPROPERTY()
	class ACharacter* OwnerCharacter;
	UPROPERTY()
	class UCharacterMovementComponent* OwnerMovementComp;

	FVector Velocity;
	float Speed;

	FRotator BodyPrevRot;
	FRotator LookRotOffset;
};
