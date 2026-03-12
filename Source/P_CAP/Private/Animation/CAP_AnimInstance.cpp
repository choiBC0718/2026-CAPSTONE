// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CAP_AnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	if (OwnerCharacter)
	{
		//
	}
}

void UCAP_AnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
}
