// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CAP_AnimInstance.h"

#include "GameFramework/Character.h"

void UCAP_AnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComp = OwnerCharacter->GetCharacterMovement();
	}
}

void UCAP_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (OwnerCharacter)
	{
		Velocity = OwnerCharacter->GetVelocity();
		Speed = Velocity.Length();
	}
}

void UCAP_AnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
}
