// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_AddGameplayTag.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UANS_AddGameplayTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                      float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());
	if (!MeshComp -> GetOwner() || !OwnerASC || !TagToApply.IsValid())
		return;

	OwnerASC->AddLooseGameplayTag(TagToApply);
}

void UANS_AddGameplayTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (AActor* Owner = MeshComp->GetOwner())
	{
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
		if (ASC && TagToApply.IsValid())
		{
			ASC->RemoveLooseGameplayTag(TagToApply);
		}
	}
}
