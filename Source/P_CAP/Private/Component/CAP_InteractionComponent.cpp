// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_InteractionComponent.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Interface/CAP_InteractInterface.h"
#include "Kismet/KismetMathLibrary.h"

UCAP_InteractionComponent::UCAP_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	NearbyInteractable = nullptr;
}

void UCAP_InteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsDialogueCameraActive)
		UpdateDialogueCamera(DeltaTime);
}

void UCAP_InteractionComponent::SetNearbyInteractable(AActor* NewActor)
{
	if (NearbyInteractable != NewActor)
	{
		NearbyInteractable = NewActor;
		if (OnInteractableChanged.IsBound())
			OnInteractableChanged.Broadcast(NearbyInteractable);
	}
}

void UCAP_InteractionComponent::ProcessInteractInput(ETriggerEvent TriggerEvent, float ElapsedTime)
{
	if (!NearbyInteractable)
		return;

	ICAP_InteractInterface* Interface = Cast<ICAP_InteractInterface>(NearbyInteractable);
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwner());
	if (!Player || !Interface)
		return;

	if (TriggerEvent == ETriggerEvent::Ongoing)
	{
		// 게이지 상승
		float Progress = FMath::Clamp(ElapsedTime/HoldThreshold , 0.f,1.f);
		OnInteractProgressUpdated.Broadcast(Progress);
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		// 1초 도달 시 분해
		Interface->Interact(Player, EInteractAction::Hold);
		OnInteractProgressUpdated.Broadcast(0.f);
	}
	else if (TriggerEvent == ETriggerEvent::Completed)
	{
		OnInteractProgressUpdated.Broadcast(0.f);
	}// 취소
	else if (TriggerEvent == ETriggerEvent::Canceled)
	{
		// 짧게 누르고 뗀 경우 Tap으로 간주, 장착
		if (ElapsedTime <= TapThreshold)
		{
			Interface->Interact(Player, EInteractAction::Tap);
		}
		OnInteractProgressUpdated.Broadcast(0.f);
	}
}

void UCAP_InteractionComponent::BeginDialogue(const struct FNPCData& InNPCData)
{	// NPC 상호작용 시 카메라 처리
	if (!NearbyInteractable)
		return;
	SetComponentTickEnabled(true);
	
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwner());
	AActor* NPC = NearbyInteractable;
	if (!Player || !NPC)
		return;

	FVector PlayerLoc = Player->GetActorLocation();
	FVector NPCLoc = NPC->GetActorLocation();

	FRotator PlayerLookAtRot = UKismetMathLibrary::FindLookAtRotation(PlayerLoc, NPCLoc);
	PlayerLookAtRot.Pitch = 0.f;
	Player->SetActorRotation(PlayerLookAtRot);

	FRotator NPCLookAtRot = UKismetMathLibrary::FindLookAtRotation(NPCLoc, PlayerLoc);
	NPCLookAtRot.Pitch = 0.f;
	NPC->SetActorRotation(NPCLookAtRot);

	USpringArmComponent* Arm = Player->GetSpringArmComponent();
	if (Arm)
	{
		OriginArmLength = Arm->TargetArmLength;
		OriginArmRotation = Arm->GetRelativeRotation();
		OriginSocketOffset = Arm->SocketOffset;

		FVector Dir = (PlayerLoc - NPCLoc).GetSafeNormal();
		FVector SideDir = FVector (-Dir.Y, Dir.X,0.f);
		float DistanceToNPC = FVector::Distance(PlayerLoc, NPCLoc);
		TargetArmLength = FMath::Max(300.f, DistanceToNPC * 1.5f);
		TargetArmRotation = SideDir.Rotation();

		TargetSocketOffset = FVector(-20.f, DistanceToNPC*0.5f, 50.f);
		bIsDialogueCameraActive = true;
	}
	OnDialogueTriggered.Broadcast(InNPCData);
}

ENPCActionResult UCAP_InteractionComponent::ExecuteNPCSpecialAction()
{
	if (NearbyInteractable)
	{
		if (ICAP_InteractInterface* InteractObj = Cast<ICAP_InteractInterface>(NearbyInteractable))
		{
			return InteractObj->ExecuteSpecialAction(GetOwner());
		}
	}
	return ENPCActionResult::Failed;
}

bool UCAP_InteractionComponent::GetNPCSpecialActionCost(int32& OutCost)
{
	if (NearbyInteractable)
	{
		if (ICAP_InteractInterface* InteractObj = Cast<ICAP_InteractInterface>(NearbyInteractable))
			return InteractObj->GetSpecialActionCost(GetOwner(), OutCost);
	}
	return false;
}

void UCAP_InteractionComponent::EndDialogueCamera()
{	// NPC 대화 종료 시 카메라 원래대로 돌려놓기
	TargetArmLength = OriginArmLength;
	TargetArmRotation = OriginArmRotation;
	TargetSocketOffset = OriginSocketOffset;
	
	bIsDialogueCameraActive = true;
	SetComponentTickEnabled(true);
}

void UCAP_InteractionComponent::UpdateDialogueCamera(float DeltaTime)
{
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwner()))
	{
		if (USpringArmComponent* Arm = Player->GetSpringArmComponent())
		{
			Arm->TargetArmLength = FMath::FInterpTo(Arm->TargetArmLength, TargetArmLength, DeltaTime,2.5f);
			Arm->SetRelativeRotation(FMath::RInterpTo(Arm->GetRelativeRotation(), TargetArmRotation, DeltaTime,2.5f));
			Arm->SocketOffset = FMath::VInterpTo(Arm->SocketOffset, TargetSocketOffset, DeltaTime,2.5f);

			bool LengthDone = FMath::IsNearlyEqual(Arm->TargetArmLength, OriginArmLength,1.f);
			bool bRotDone = Arm->GetRelativeRotation().Equals(TargetArmRotation, 0.5f);
			bool bOffsetDone = Arm->SocketOffset.Equals(TargetSocketOffset, 1.f);

			if (LengthDone && bRotDone && bOffsetDone)
			{
				Arm->TargetArmLength = TargetArmLength;
				Arm->SetRelativeRotation(TargetArmRotation);
				Arm->SocketOffset = TargetSocketOffset;

				bIsDialogueCameraActive = false;
				SetComponentTickEnabled(false);
			}
		}
	}
}

