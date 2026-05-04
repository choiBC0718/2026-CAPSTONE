// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_InteractionComponent.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Interface/CAP_InteractInterface.h"

UCAP_InteractionComponent::UCAP_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	NearbyInteractable = nullptr;
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

