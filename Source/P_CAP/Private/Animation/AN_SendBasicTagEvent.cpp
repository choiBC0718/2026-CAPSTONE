// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_SendBasicTagEvent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"

void UAN_SendBasicTagEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (!MeshComp -> GetOwner() || !OwnerASC)
		return;

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	Payload.OptionalObject = this;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp -> GetOwner(), Payload.EventTag, Payload);
}

FString UAN_SendBasicTagEvent::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
		return TagNames.Last().ToString();
	}
	return "None";
}
