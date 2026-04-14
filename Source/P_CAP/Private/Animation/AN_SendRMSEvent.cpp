// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_SendRMSEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAN_SendRMSEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (!MeshComp || !MeshComp->GetOwner())
		return;
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;

	AActor* OwnerActor = MeshComp->GetOwner();
	if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor) == nullptr)
		return;
	

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.OptionalObject = this;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
}

FString UAN_SendRMSEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
		return TagNames.Last().ToString();
	}
	return "None";
}
