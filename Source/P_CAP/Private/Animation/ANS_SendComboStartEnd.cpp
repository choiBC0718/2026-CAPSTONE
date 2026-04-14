// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_SendComboStartEnd.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"

UANS_SendComboStartEnd::UANS_SendComboStartEnd()
{
	TargetClearTag = UCAP_AbilitySystemStatics::GetTargetClearTag();
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

	if (NextComboNameTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), NextComboNameTag, FGameplayEventData());
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

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), TargetClearTag, FGameplayEventData());
	
	if (ComboEndTag.IsValid())
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), ComboEndTag, FGameplayEventData());
}
