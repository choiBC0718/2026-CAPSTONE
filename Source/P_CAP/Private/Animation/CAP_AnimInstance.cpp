// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/CAP_AnimInstance.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "GameFramework/Character.h"
#include "Items/Weapon/CAP_WeaponInstance.h"

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

		if (APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController()))
		{
			FHitResult HitResult;

			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
			{
				FVector TargetLoc = HitResult.ImpactPoint;
				FVector ActorLoc = OwnerCharacter->GetActorLocation();
				
				TargetLoc.Z = ActorLoc.Z;

				// 마우스가 캐릭터와 너무 가까울 때 회전값이 튀는 것(NaN)을 방지
				if ((TargetLoc - ActorLoc).SizeSquared() > 100.f)
				{
					// 1. 캐릭터에서 마우스 위치를 바라보는 '절대 회전값'
					FRotator LookAtRot = (TargetLoc - ActorLoc).Rotation();
					
					// 2. 캐릭터가 현재 바라보는 '기준 회전값'
					FRotator ActorRot = OwnerCharacter->GetActorRotation();

					// 3. 두 회전값의 차이를 구하고 -180 ~ 180도로 정규화(Normalize)
					FRotator DeltaRot = LookAtRot - ActorRot;
					DeltaRot.Normalize();

					FRotator CurrentAimRot(0.f, AimYaw, 0.f);
					FRotator TargetAimRot(0.f, DeltaRot.Yaw, 0.f);
					
					AimYaw = FMath::RInterpTo(CurrentAimRot, TargetAimRot, DeltaSeconds, 15.f).Yaw;
				}
			}
		}
	}
}

void UCAP_AnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
}

void UCAP_AnimInstance::UpdateWeaponAnimData(class UCAP_WeaponDataAsset* WeaponDA)
{
	if (WeaponDA)
	{
		CurrentIdleAnim = WeaponDA->IdleAnim;
		CurrentJogStartAnim = WeaponDA->JogStartAnim;
		CurrentJoggingAnim = WeaponDA->JoggingAnim;
		CurrentJogEndAnim = WeaponDA->JogEndAnim;

		CurrentJogEndStartTime = WeaponDA->JogEndStartTime;
	}
}
