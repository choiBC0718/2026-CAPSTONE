// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AbilityTask_RotateToCursor.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UAbilityTask_RotateToCursor::UAbilityTask_RotateToCursor()
{
	bTickingTask = true;
	bIsFinished = false;
}

void UAbilityTask_RotateToCursor::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	if (bIsFinished) return;

	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character) return;

	FRotator CurrentRot = Character->GetActorRotation();
	
	FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, TargetRotation, DeltaTime, InterpSpeed);
	Character->SetActorRotation(NewRot);

	// 목표 각도에 거의 도달했다면 회전 종료
	if (CurrentRot.Equals(TargetRotation, 1.f))
	{
		bIsFinished = true;
		OnComplete.Broadcast();
		EndTask();
	}
}

void UAbilityTask_RotateToCursor::Activate()
{
	Super::Activate();

	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character) return;
	
	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (PC)
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
		{
			FVector StartLoc = Character->GetActorLocation();
			FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(StartLoc, HitResult.ImpactPoint);
			TargetRotation = FRotator(0.f, LookAtRot.Yaw, 0.f); 
		}
		else
		{
			// 바닥을 못 찍었을 경우 제자리 유지
			TargetRotation = FRotator(0.f, Character->GetActorRotation().Yaw, 0.f); 
		}
	}
}

UAbilityTask_RotateToCursor* UAbilityTask_RotateToCursor::SmoothRotateToMouse(UGameplayAbility* OwningAbility,	float RotationSpeed)
{
	UAbilityTask_RotateToCursor* MyObj = NewAbilityTask<UAbilityTask_RotateToCursor>(OwningAbility);
	MyObj->InterpSpeed = RotationSpeed;
	return MyObj;
}
