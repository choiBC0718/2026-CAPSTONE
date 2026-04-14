// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Character.h"
#include "CAP_Character.generated.h"

UCLASS()
class ACAP_Character : public ACharacter, public IAbilitySystemInterface
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

protected:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY(EditDefaultsOnly, Category="Stat")
	FName CharacterStatRowName = "Player";

public:
	bool IsDead() const;
	
private:
	void BindGASChangeDelegates();
	
	void DeadTagUpdated(FGameplayTag GameplayTag, int32 NewCount);
	void MoveSpeedUpdated(const FOnAttributeChangeData& OnAttributeChangeData);
	void MaxHealthUpdated(const FOnAttributeChangeData& OnAttributeChangeData);

	FTransform MeshRelativeTransform;
	
	void StartDeathSequence();
	virtual void OnDead();
	void PlayDeathAnimation();
	void DeathMontageFinished();
	void SetRagdollEnabled(bool bIsEnable);

	UPROPERTY(EditDefaultsOnly, Category="Death")
	float DeathMontageFinishTimeShift = -0.8f;
	UPROPERTY(EditDefaultsOnly, Category="Death")
	UAnimMontage* DeathMontage;
	FTimerHandle DeathMontageTimerHandle;
	
	void Respawn();
	virtual void OnRespawn();
	void SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled);
};
