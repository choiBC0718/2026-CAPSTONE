// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Tasks/AbilityTask_TickRotToCursor.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UAbilityTask_TickRotToCursor::UAbilityTask_TickRotToCursor()
{
	bTickingTask=true;
}

void UAbilityTask_TickRotToCursor::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
	if (!Character) return;

	FRotator CurrentRot = Character->GetActorRotation();

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
	
	FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, TargetRotation, DeltaTime, InterpSpeed);
	Character->SetActorRotation(NewRot);
}

UAbilityTask_TickRotToCursor* UAbilityTask_TickRotToCursor::TickRotToCursor(UGameplayAbility* OwningAbility,float RotationSpeed)
{
	UAbilityTask_TickRotToCursor* MyObj = NewAbilityTask<UAbilityTask_TickRotToCursor>(OwningAbility);
	MyObj->InterpSpeed = RotationSpeed;
	return MyObj;
}
