// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/CAP_Character.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "P_CAP/P_CAP.h"

ACAP_Character::ACAP_Character()
{
	PrimaryActorTick.bCanEverTick = false;

	GetMesh()->SetupAttachment(GetRootComponent());
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target,ECR_Ignore);

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


UAbilitySystemComponent* ACAP_Character::GetAbilitySystemComponent() const
{
	return CAPAbilitySystemComponent;
}

bool ACAP_Character::IsDead() const
{
	return GetAbilitySystemComponent()->HasMatchingGameplayTag(UCAP_AbilitySystemStatics::GetDeadStateTag());
}

void ACAP_Character::BindGASChangeDelegates()
{
	if (CAPAbilitySystemComponent)
	{
		CAPAbilitySystemComponent->RegisterGameplayTagEvent(UCAP_AbilitySystemStatics::GetDeadStateTag()).AddUObject(this, &ACAP_Character::DeadTagUpdated);
		
		CAPAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAP_AttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &ACAP_Character::MoveSpeedUpdated);
		CAPAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAP_AttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ACAP_Character::MaxHealthUpdated);
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
		Respawn();
	}
}

void ACAP_Character::MoveSpeedUpdated(const FOnAttributeChangeData& OnAttributeChangeData)
{
	GetCharacterMovement()->MaxWalkSpeed = OnAttributeChangeData.NewValue;
}

void ACAP_Character::MaxHealthUpdated(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (IsValid(CAPAttributeSet))
		CAPAttributeSet->RescaleHealth();
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
void ACAP_Character::OnDead(){}

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
		SetRagdollEnabled(true);
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

void ACAP_Character::OnRespawn(){}

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

