// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Character.h"
#include "Interface/CAP_TargetUIInterface.h"
#include "CAP_Character.generated.h"

UCLASS()
class ACAP_Character : public ACharacter, public IAbilitySystemInterface, public ICAP_TargetUIInterface
{
	GENERATED_BODY()

public:
	ACAP_Character();
	virtual void BeginPlay() override;

private:
	/**		Components		**/
	UPROPERTY(VisibleAnywhere, Category="Ability")
	class UCAP_AbilitySystemComponent* CAPAbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, Category="Ability")
	class UCAP_AttributeSet* CAPAttributeSet;
	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;
	
	virtual void UpdateStackUI(const FGameplayTag& BehaviorTag, int32 CurrentStack, int32 MaxStack) override {}

protected:
	UPROPERTY(VisibleAnywhere, Category="Widget")
	class UWidgetComponent* TargetEffectWidgetComp;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditDefaultsOnly, Category="Stat")
	FName CharacterStatRowName = "Player";
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Death")
	bool bCanRespawn = true;

public:
	bool IsDead() const;
	bool IsAlive() const;
	
private:
	void BindGASChangeDelegates();
	
	void DeadTagUpdated(FGameplayTag GameplayTag, int32 NewCount);
	void MoveSpeedUpdated(const FOnAttributeChangeData& OnAttributeChangeData);

	FTransform MeshRelativeTransform;

protected:
	void StartDeathSequence();
	virtual void OnDead() {};
	void PlayDeathAnimation();
	virtual void DeathMontageFinished();
	void SetRagdollEnabled(bool bIsEnable);

	UPROPERTY(EditDefaultsOnly, Category="Death")
	float DeathMontageFinishTimeShift = -0.8f;
	UPROPERTY(EditDefaultsOnly, Category="Death")
	UAnimMontage* DeathMontage;
	FTimerHandle DeathMontageTimerHandle;
	
	void Respawn();
	virtual void OnRespawn() {};
	void SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled);
};
