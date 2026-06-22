// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "Interface/CAP_TargetUIInterface.h"
#include "CAP_Character.generated.h"

UCLASS()
class ACAP_Character : public ACharacter, public IAbilitySystemInterface, public ICAP_TargetUIInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ACAP_Character();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/**		Components		**/
	UPROPERTY(VisibleAnywhere, Category="Ability")
	class UCAP_AbilitySystemComponent* CAPAbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, Category="Ability")
	class UCAP_AttributeSet* CAPAttributeSet;
	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;

	// ICAP_TargetUIInterface
	virtual void UpdateStackUI(const FGameplayTag& BehaviorTag, int32 CurrentStack, int32 MaxStack) override {}

	// IGenericTeamAgentInterface
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

protected:
	UPROPERTY(VisibleAnywhere, Category="Widget")
	class UWidgetComponent* TargetEffectWidgetComp;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditDefaultsOnly, Category="Stat")
	FName CharacterStatRowName = "Player";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	bool bCanRespawn = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
	FGenericTeamId TeamId = FGenericTeamId::NoTeam;
public:
	bool IsDead() const;
	bool IsAlive() const;
	
private:
	void BindGASChangeDelegates();
	
	void DeadTagUpdated(FGameplayTag GameplayTag, int32 NewCount);
	void MoveSpeedUpdated(const FOnAttributeChangeData& OnAttributeChangeData);

	FTransform MeshRelativeTransform;
	
	void StartDeathSequence();
	virtual void OnDead() {};
	void PlayDeathAnimation();
protected:
	virtual void DeathMontageFinished();
	virtual void OnDeathMontageFinished() {};
	void SetRagdollEnabled(bool bIsEnable);

	UPROPERTY(EditDefaultsOnly, Category="Death")
	float DeathMontageFinishTimeShift = -0.8f;
	UPROPERTY(EditDefaultsOnly, Category="Death")
	UAnimMontage* DeathMontage;
	FTimerHandle DeathMontageTimerHandle;
	
	void Respawn();
	virtual void OnRespawn() {};
	void SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled);

public:
	virtual void PlayHitFeedback();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="Feedback")
	class UMaterialInterface* HitOverlayMaterial;
	UPROPERTY(EditDefaultsOnly, Category="Feedback")
	float HitFlashDuration = 0.1f;

	FTimerHandle HitFlashTimerHandle;
	void StopHitFeedback();
};
