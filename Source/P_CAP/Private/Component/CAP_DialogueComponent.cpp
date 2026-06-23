// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_DialogueComponent.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Kismet/KismetMathLibrary.h"

UCAP_DialogueComponent::UCAP_DialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UCAP_DialogueComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsCameraMoving)
		UpdateCamera(DeltaTime);
}

void UCAP_DialogueComponent::BeginDialogue(class ACAP_PlayerCharacter* InPlayer)
{
	if (!InPlayer || !GetOwner())
		return;
	Player = InPlayer;

	AActor* NPC = GetOwner();
	FVector PlayerLoc = Player->GetActorLocation();
	FVector NPCLoc = NPC->GetActorLocation();

	FRotator PlayerLookAtRot = UKismetMathLibrary::FindLookAtRotation(PlayerLoc, NPCLoc);
	PlayerLookAtRot.Pitch = 0.f;
	Player->SetActorRotation(PlayerLookAtRot);
	
	FRotator NPCLookAtRot = UKismetMathLibrary::FindLookAtRotation(NPCLoc, PlayerLoc);
	NPCLookAtRot.Pitch = 0.f;
	NPC->SetActorRotation(NPCLookAtRot);

	if (USpringArmComponent* Arm = Player->GetSpringArmComponent())
	{
		// 원래 카메라 위치 저장
		OriginArmLength = Arm->TargetArmLength;
		OriginArmRotation = Arm->GetRelativeRotation();
		OriginSocketOffset = Arm->SocketOffset;

		FVector Dir = (PlayerLoc - NPCLoc).GetSafeNormal();
		FVector SideDir = FVector(-Dir.Y, Dir.X, 0.f);
		float DistanceToNPC = FVector::Distance(PlayerLoc, NPCLoc);

		// 이동될 카메라 위치
		TargetArmLength = FMath::Max(300.f, DistanceToNPC * 1.5f);
		TargetArmRotation = SideDir.Rotation();
		TargetSocketOffset = FVector(-20.f, DistanceToNPC * 0.5f, 50.f);

		bIsCameraMoving = true;
		SetComponentTickEnabled(true);
	}
}

void UCAP_DialogueComponent::EndDialogue()
{
	if (!Player)
		return;

	TargetArmLength = OriginArmLength;
	TargetArmRotation = OriginArmRotation;
	TargetSocketOffset = OriginSocketOffset;

	bIsCameraMoving = true;
	SetComponentTickEnabled(true);
	
	if (FSlateApplication::IsInitialized())
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

ENPCActionResult UCAP_DialogueComponent::ExecuteSpecialAction(AActor* InteractActor)
{
	if (OnExecuteSpecialAction.IsBound())
	{
		return OnExecuteSpecialAction.Execute(InteractActor);
	}
	return ENPCActionResult::Failed;
}

bool UCAP_DialogueComponent::GetSpecialActionCost(int32& OutCost)
{
	if (OnGetSpecialActionCost.IsBound())
		return OnGetSpecialActionCost.Execute(OutCost);
	return false;
}

void UCAP_DialogueComponent::UpdateCamera(float DeltaTime)
{
	if (!Player || !bIsCameraMoving)
		return;

	if (USpringArmComponent* Arm = Player->GetSpringArmComponent())
	{
		Arm->TargetArmLength = FMath::FInterpTo(Arm->TargetArmLength, TargetArmLength, DeltaTime,2.5f);
		Arm->SetRelativeRotation(FMath::RInterpTo(Arm->GetRelativeRotation(), TargetArmRotation, DeltaTime,2.5f));
		Arm->SocketOffset = FMath::VInterpTo(Arm->SocketOffset, TargetSocketOffset, DeltaTime,2.5f);

		if (FMath::IsNearlyEqual(Arm->TargetArmLength, TargetArmLength, 1.f))
		{
			bIsCameraMoving = false;
			SetComponentTickEnabled(false);
		}
	}
}
