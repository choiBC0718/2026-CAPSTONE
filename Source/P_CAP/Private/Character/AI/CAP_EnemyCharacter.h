// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/CAP_Character.h"
#include "CAP_EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_EnemyCharacter : public ACAP_Character
{
	GENERATED_BODY()

public:
	ACAP_EnemyCharacter();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Enemy|AI")
	void SetEnemyAIEnabled(bool bEnabled, AActor* TargetActor = nullptr);

	UFUNCTION(BlueprintPure, Category="Enemy|AI")
	bool IsEnemyAIEnabled() const { return bEnemyAIEnabled; }

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Enemy|Room")
	void OnRoomActivated(AActor* TargetActor);
	virtual void OnRoomActivated_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Enemy|Room")
	void OnRoomDeactivated();
	virtual void OnRoomDeactivated_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Enemy|Combat")
	void PerformAttack(AActor* TargetActor);
	virtual void PerformAttack_Implementation(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category="Enemy|Combat")
	bool TryPerformAttack(AActor* TargetActor);

	UFUNCTION(BlueprintPure, Category="Enemy|Combat")
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure, Category="Enemy|Combat")
	float GetAttackCooldown() const { return AttackCooldown; }

protected:
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnDead() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Combat", meta=(ClampMin="0.0"))
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Combat", meta=(ClampMin="0.0"))
	float AttackCooldown = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Enemy|Combat", meta=(ClampMin="0.0"))
	float AttackDamage = 10.f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Enemy|AI")
	bool bEnemyAIEnabled = false;

private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentTargetActor;

	double LastAttackTime = -1000.0;
};
