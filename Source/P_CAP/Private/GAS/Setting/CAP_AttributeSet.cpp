// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/CAP_AttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CAP_AbilitySystemStatics.h"
#include "CAP_GameplayAbilityTypes.h"
#include "GameplayEffectExtension.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Framework/Subsystem/CAP_DamageTextSubsystem.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Kismet/GameplayStatics.h"

UCAP_AttributeSet::UCAP_AttributeSet()
{
	DagameTakenTag = UCAP_AbilitySystemStatics::GetItemTriggerDamageTaken();
	HealTag = UCAP_AbilitySystemStatics::GetItemTriggerHealed();
}

void UCAP_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetMaxHealthAttribute())
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
}

void UCAP_AttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* Target = GetOwningActor();
	AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();

	bool bIsTargetPlayer = false;	// Target (피격자)
	if (IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(Target))
		bIsTargetPlayer = (TargetTeamAgent->GetGenericTeamId().GetId() == 0);
	
	bool bIsInstigatorPlayer = false;	// Instigator (공격자)
	if (IGenericTeamAgentInterface* InstigatorTeamAgent = Cast<IGenericTeamAgentInterface>(Instigator))
		bIsInstigatorPlayer = (InstigatorTeamAgent->GetGenericTeamId().GetId() == 0);
	
	if (!CachedProgressionSubsystem && Target)
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(Target))
			CachedProgressionSubsystem = GI->GetSubsystem<UCAP_ProgressionSubsystem>();
	}

	if (!CachedProgressionSubsystem && Target)
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(Target))
			CachedProgressionSubsystem = GI->GetSubsystem<UCAP_ProgressionSubsystem>();
	}
	
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		float LocalDamage = GetDamage();
		SetDamage(0.f);
		if (LocalDamage > 0.f)
		{
			if (CachedProgressionSubsystem)		// [대시보드 통계 누적]
			{
				if (bIsTargetPlayer)			// 플레이어가 맞은 경우
					CachedProgressionSubsystem->AddDamageTaken(LocalDamage);
				else if (bIsInstigatorPlayer)	// 플레이어가 때린 경우
					CachedProgressionSubsystem->AddDamageDeal(LocalDamage);
			}

			FGameplayEventData HitPayload;
			HitPayload.EventTag = FGameplayTag::RequestGameplayTag("Item.Trigger.Hit.Taken");
			HitPayload.Instigator = Instigator;
			HitPayload.Target = Target;
			HitPayload.EventMagnitude = LocalDamage;
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, HitPayload.EventTag, HitPayload);
			
			if (GetShield() > 0.f)
			{	// 방어막 먼저 깎기
				float ShieldDamage = FMath::Min(GetShield(), LocalDamage);
				SetShield(GetShield() - ShieldDamage);
				LocalDamage -= ShieldDamage;
			}
			if (LocalDamage >0.f)
			{
				const float NewHealth = GetHealth() - LocalDamage;
				SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

				bool bIsCritical = false;
				if (const FCAP_GameplayEffectContext* CContext = static_cast<const FCAP_GameplayEffectContext*>(Data.EffectSpec.GetContext().Get()))
					bIsCritical = CContext->bIsCritical;
				
				if (Target)
				{
					if (UCAP_DamageTextSubsystem* TextSubsys = Target->GetWorld()->GetSubsystem<UCAP_DamageTextSubsystem>())
						TextSubsys->ShowDamage(Target,LocalDamage,bIsCritical,bIsTargetPlayer);
					
				}
				if (ACAP_Character* TargetChar = Cast<ACAP_Character>(Target))
					TargetChar->PlayHitFeedback();
			}
		}
	}
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		float DeltaValue = Data.EvaluatedData.Magnitude;
		if (DeltaValue > 0.f)
		{
			bool bIsSystemHeal = Data.EffectSpec.Def->GetAssetTags().HasTag(FGameplayTag::RequestGameplayTag("Effect.IgnoreStatistic"));
			if (!bIsSystemHeal)
			{
				if (CachedProgressionSubsystem && bIsTargetPlayer)
					CachedProgressionSubsystem->AddHealing(DeltaValue);
				
				FGameplayEventData HealPayload;
				HealPayload.EventTag = FGameplayTag::RequestGameplayTag(FName("Event.Status.Healed"));
				HealPayload.Instigator = Instigator;
				HealPayload.Target = Target;
				HealPayload.EventMagnitude = DeltaValue;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, HealPayload.EventTag, HealPayload);
			}
		}
		SetHealth(FMath::Clamp(GetHealth(), 0, GetMaxHealth()));
	}
}

void UCAP_AttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
	const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	const float CurrentMaxVal = MaxAttribute.GetCurrentValue();

	if (CurrentMaxVal <=0.f || !ASC) return;
	if (!FMath::IsNearlyEqual(CurrentMaxVal, NewMaxValue) && ASC)
	{
		const float CurrentVal = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxVal > 0.f)? (CurrentVal * NewMaxValue / CurrentMaxVal)-CurrentVal : NewMaxValue;
		ASC->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}