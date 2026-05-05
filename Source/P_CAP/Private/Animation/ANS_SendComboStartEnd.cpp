// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_SendComboStartEnd.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

UANS_SendComboStartEnd::UANS_SendComboStartEnd()
{
	NextComboTag = FGameplayTag::RequestGameplayTag("Ability.Combo");
	ComboEndTag = FGameplayTag::RequestGameplayTag("Ability.Combo.End");
}

void UANS_SendComboStartEnd::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp->GetOwner())
		return;

	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
		return;

	FGameplayEventData Payload;
	Payload.OptionalObject = this;
	Payload.EventTag = NextComboTag;
	
	if (NextComboTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), NextComboTag, Payload);
}

void UANS_SendComboStartEnd::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!MeshComp->GetOwner())
		return;

	if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()))
		return;
	
	if (ComboEndTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), ComboEndTag, FGameplayEventData());
}
