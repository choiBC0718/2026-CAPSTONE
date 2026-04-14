// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Tasks/AbilityTask_TickMoveToCursor.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Kismet/KismetMathLibrary.h"

UAbilityTask_TickMoveToCursor::UAbilityTask_TickMoveToCursor()
{
	bIsFinished=false;
	bTickingTask=true;
	SpeedMultiplier=1.f;
	OriginalMaxWalkSpeed=0.f;
}

void UAbilityTask_TickMoveToCursor::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bIsFinished)
		return;

	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character)
		return;
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC)
		return;

	FHitResult HitReuslt;
	if (PC->GetHitResultUnderCursor(ECC_Vehicle, false, HitReuslt))
	{
		FVector StartLoc = Character->GetActorLocation();
		FVector TargetLoc = HitReuslt.ImpactPoint;

		StartLoc.Z = 0.f;
		TargetLoc.Z = 0.f;
		FVector Dir = (TargetLoc - StartLoc).GetSafeNormal();
		float Distance = FVector::Distance(StartLoc, TargetLoc);

		if (Distance> 50.f)
		{
			if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
			{
				float CurrentMoveSpeed = 500.f;
				bool bFound = false;
				CurrentMoveSpeed = ASC->GetGameplayAttributeValue(UCAP_AttributeSet::GetMoveSpeedAttribute(), bFound);
				Character->GetCharacterMovement()->MaxWalkSpeed = CurrentMoveSpeed;
			}
			Character->AddMovementInput(Dir, 1.f);

			FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(StartLoc, TargetLoc);
			FRotator CurrentRot = Character->GetActorRotation();
			// 회전 속도 800.f는 입맛에 맞게 조절하세요.
			FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, FRotator(0.f, LookAtRot.Yaw, 0.f), DeltaTime, 800.f); 
			Character->SetActorRotation(NewRot);
		}
	}
}

void UAbilityTask_TickMoveToCursor::Activate()
{
	Super::Activate();
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character && Character->GetCharacterMovement())
	{
		OriginalMaxWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
	}
}

void UAbilityTask_TickMoveToCursor::OnDestroy(bool bInOwnerFinished)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (Character && Character->GetCharacterMovement() && OriginalMaxWalkSpeed > 0.f)
	{
		// 태스크가 끝나면 캐릭터의 속도를 스킬 쓰기 전으로 원상 복구합니다.
		Character->GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
	}
	Super::OnDestroy(bInOwnerFinished);
}

UAbilityTask_TickMoveToCursor* UAbilityTask_TickMoveToCursor::MoveToCursor(UGameplayAbility* OwningAbility,
                                                                           float InSpeedMultiplier)
{
	UAbilityTask_TickMoveToCursor* MyObj = NewAbilityTask<UAbilityTask_TickMoveToCursor>(OwningAbility);
	MyObj->SpeedMultiplier=InSpeedMultiplier;
	return MyObj;
}
