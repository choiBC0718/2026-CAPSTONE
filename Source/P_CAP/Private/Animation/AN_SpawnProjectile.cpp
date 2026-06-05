// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AN_SpawnProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTagsManager.h"
#include "Abilities/GameplayAbilityTypes.h"

UAN_SpawnProjectile::UAN_SpawnProjectile()
{
	EventTag = FGameplayTag::RequestGameplayTag("Ability.Event.AnimSpawn");
}

void UAN_SpawnProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
		return;
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;

	FGameplayEventData EventData;
	EventData.EventTag=EventTag;
	EventData.OptionalObject=this;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), EventTag,EventData);
}

FString UAN_SpawnProjectile::GetNotifyName_Implementation() const
{
	if (EventTag.IsValid())
	{
		TArray<FName> TagNames;
		UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag, TagNames);
		return TagNames.Last().ToString();
	}
	return "None";
}
