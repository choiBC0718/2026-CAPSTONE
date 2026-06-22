// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CAP_Character.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

ACAP_Character::ACAP_Character()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetupAttachment(GetRootComponent());
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetReceivesDecals(false);

	TargetEffectWidgetComp=CreateDefaultSubobject<UWidgetComponent>("StackEffectWidget");
	TargetEffectWidgetComp->SetupAttachment(GetRootComponent());
	TargetEffectWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TargetEffectWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, -60.f));
	TargetEffectWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);

	CAPAbilitySystemComponent = CreateDefaultSubobject<UCAP_AbilitySystemComponent>("Ability System Component");
	CAPAttributeSet = CreateDefaultSubobject<UCAP_AttributeSet>("Attribute Set");

	BindGASChangeDelegates();

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception StimuliSource Component");
}

void ACAP_Character::BeginPlay()
{
	Super::BeginPlay();

	MeshRelativeTransform = GetMesh() -> GetRelativeTransform();
	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

void ACAP_Character::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HitFlashTimerHandle.IsValid())
		GetWorldTimerManager().ClearTimer(HitFlashTimerHandle);
	Super::EndPlay(EndPlayReason);
}


void ACAP_Character::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	TeamId = TeamID;
}

FGenericTeamId ACAP_Character::GetGenericTeamId() const
{
	return TeamId;
}

ETeamAttitude::Type ACAP_Character::GetTeamAttitudeTowards(const AActor& Other) const
{
	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(&Other);
	
	if (!OtherTeamAgent)
	{
		if (const APawn* OtherPawn = Cast<APawn>(&Other))
		{
			OtherTeamAgent = Cast<IGenericTeamAgentInterface>(OtherPawn->GetController());
		}
	}
	
	if (OtherTeamAgent)
	{
		FGenericTeamId OtherTeamID = OtherTeamAgent->GetGenericTeamId();
		
		if (OtherTeamID == FGenericTeamId::NoTeam || TeamId == FGenericTeamId::NoTeam)
			return ETeamAttitude::Neutral;
		
		if (OtherTeamID == TeamId)
			return ETeamAttitude::Friendly;
		else
			return ETeamAttitude::Hostile;
		
	}

	return ETeamAttitude::Neutral;
}

UAbilitySystemComponent* ACAP_Character::GetAbilitySystemComponent() const
{
	return CAPAbilitySystemComponent;
}

bool ACAP_Character::IsDead() const
{
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(UCAP_AbilitySystemStatics::GetDeadStateTag());
}

bool ACAP_Character::IsAlive() const
{
	return IsValid(CAPAttributeSet) && CAPAttributeSet->GetHealth() > 0.f;
}

void ACAP_Character::BindGASChangeDelegates()
{
	if (CAPAbilitySystemComponent)
	{
		CAPAbilitySystemComponent->RegisterGameplayTagEvent(UCAP_AbilitySystemStatics::GetDeadStateTag()).AddUObject(this, &ACAP_Character::DeadTagUpdated);
		
		CAPAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAP_AttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &ACAP_Character::MoveSpeedUpdated);
	}
}

void ACAP_Character::DeadTagUpdated(FGameplayTag GameplayTag, int32 NewCount)
{
	if (NewCount != 0)
	{
		StartDeathSequence();
	}
	else
	{
		if (bCanRespawn)
		{
			Respawn();
		}
	}
}

void ACAP_Character::MoveSpeedUpdated(const FOnAttributeChangeData& OnAttributeChangeData)
{
	GetCharacterMovement()->MaxWalkSpeed = OnAttributeChangeData.NewValue;
}

void ACAP_Character::StartDeathSequence()
{
	OnDead();

	if (CAPAbilitySystemComponent)
		CAPAbilitySystemComponent->CancelAllAbilities();

	PlayDeathAnimation();
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetAIPerceptionStimuliSourceEnabled(false);
}

void ACAP_Character::Respawn()
{
	OnRespawn();
	SetRagdollEnabled(false);
	GetCharacterMovement() -> SetMovementMode(EMovementMode::MOVE_Walking);
	GetCapsuleComponent() -> SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetAIPerceptionStimuliSourceEnabled(true);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);

	if (CAPAbilitySystemComponent)
	{
		CAPAbilitySystemComponent->ApplyFullStatEffect();
	}
}

void ACAP_Character::PlayDeathAnimation()
{
	if (DeathMontage)
	{
		float MontageDuration = PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &ACAP_Character::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
}

void ACAP_Character::DeathMontageFinished()
{
	if (IsDead())
	{
		SetRagdollEnabled(true);
		OnDeathMontageFinished();
	}
}

void ACAP_Character::SetRagdollEnabled(bool bIsEnable)
{
	if (bIsEnable)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void ACAP_Character::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if (!PerceptionStimuliSourceComponent)
		return;

	if (bIsEnabled)
	{
		PerceptionStimuliSourceComponent -> RegisterWithPerceptionSystem();		//true : 기능 등록
	}
	else
	{
		PerceptionStimuliSourceComponent -> UnregisterFromPerceptionSystem();	//false : 기능 해제
	}
}

void ACAP_Character::PlayHitFeedback()
{
	if (!HitOverlayMaterial || !GetMesh())
		return;
	GetMesh()->SetOverlayMaterial(HitOverlayMaterial);

	GetWorldTimerManager().SetTimer(HitFlashTimerHandle, this, &ACAP_Character::StopHitFeedback, HitFlashDuration, false);
}

void ACAP_Character::StopHitFeedback()
{
	if (GetMesh())
		GetMesh()->SetOverlayMaterial(nullptr);
}

