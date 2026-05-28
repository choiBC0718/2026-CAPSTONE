// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_InteractionComponent.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Interface/CAP_InteractInterface.h"

UCAP_InteractionComponent::UCAP_InteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	NearbyInteractable = nullptr;
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(GetOwner()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			InvComp->OnInventoryFull.AddDynamic(this, &UCAP_InteractionComponent::HandleInventoryFull);
	}
}

void UCAP_InteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateNearbyInteractable();
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

// 상호작용 인터페이스 메소드의 Interact, CAP_WorldNPC가 override하여 호출되는 부분
void UCAP_InteractionComponent::BeginDialogue(const struct FNPCData& InNPCData)
{
	if (NearbyInteractable)
	{
		if (UCAP_DialogueComponent* DialogueComp = NearbyInteractable->FindComponentByClass<UCAP_DialogueComponent>())
		{
			bIsInDialogue = true;
			// NPC의 Dialogue Component를 통해 카메라 시점 전환
			DialogueComp->BeginDialogue(Cast<ACAP_PlayerCharacter>(GetOwner()));
			// 대화 시작 방송 -> Dialogue Widget이 구독하여 대화 시작 이벤트 준비
			OnDialogueTriggered.Broadcast(InNPCData);
		}
	}
}

ENPCActionResult UCAP_InteractionComponent::ExecuteNPCSpecialAction()
{
	if (NearbyInteractable)
	{
		if (UCAP_DialogueComponent* DialogueComp = NearbyInteractable->FindComponentByClass<UCAP_DialogueComponent>())
		{
			return DialogueComp->ExecuteSpecialAction(GetOwner());
		}
	}
	return ENPCActionResult::Failed;
}

bool UCAP_InteractionComponent::GetNPCSpecialActionCost(int32& OutCost)
{
	if (NearbyInteractable)
	{
		if (UCAP_DialogueComponent* DialogueComp = NearbyInteractable->FindComponentByClass<UCAP_DialogueComponent>())
			return DialogueComp->GetSpecialActionCost(OutCost);
	}
	return false;
}

void UCAP_InteractionComponent::EndDialogueCamera()
{	
	if (NearbyInteractable)
		{
			if (UCAP_DialogueComponent* DialogueComp = NearbyInteractable->FindComponentByClass<UCAP_DialogueComponent>())
			{
				DialogueComp->EndDialogue();
			}
		}
		
		bIsInDialogue = false;
		SetNearbyInteractable(nullptr);
		UpdateNearbyInteractable();
}

void UCAP_InteractionComponent::HandleInventoryFull(class UCAP_ItemInstance* OverflowItem)
{
	LastInventoryFullActor = NearbyInteractable;
	SetNearbyInteractable(nullptr);
	SetComponentTickEnabled(false);
}

void UCAP_InteractionComponent::AddInteractable(AActor* NewActor)
{
	if (NewActor && !OverlappedInteractable.Contains(NewActor))
	{
		OverlappedInteractable.Add(NewActor);
		
		if (OverlappedInteractable.Num() > 0 && !IsComponentTickEnabled())
		{
			SetComponentTickEnabled(true);
		}
		UpdateNearbyInteractable();
	}
}

void UCAP_InteractionComponent::RemoveInteractable(AActor* TargetActor)
{
	if (TargetActor && OverlappedInteractable.Contains(TargetActor))
	{
		OverlappedInteractable.Remove(TargetActor);

		if (OverlappedInteractable.IsEmpty())
		{
			SetComponentTickEnabled(false);
			SetNearbyInteractable(nullptr);
		}
	}
}

void UCAP_InteractionComponent::UpdateNearbyInteractable()
{
	if (bIsInDialogue)
		return;
	if (OverlappedInteractable.IsEmpty())
	{
		SetNearbyInteractable(nullptr);
		return;
	}

	AActor* ClosestActor = nullptr;
	float MinDist = TNumericLimits<float>::Max();
	FVector PlayerLoc = GetOwner()->GetActorLocation();

	for (AActor* Actor : OverlappedInteractable)
	{
		if (Actor)
		{
			float Dist = FVector::DistSquared(PlayerLoc, Actor->GetActorLocation());
			if (Dist<MinDist)
			{
				MinDist = Dist;
				ClosestActor = Actor;
			}
		}
	}
	SetNearbyInteractable(ClosestActor);
}
