// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "CAP_AttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class UCAP_AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	void RescaleHealth();

	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, PhysicalDamage);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, PhysicalPenetration);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MagicalDamage);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MagicalPenetration);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, PhysicalArmor);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MagicalArmor);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, CriticalChance);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, CriticalDamage);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, AttackSpeed);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MoveSpeed);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, SkillCooldownSpeed);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, WeaponSwapSpeed);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, CurrentDodgeCount);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MaxDodgeCount);

	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, CachedHealthPercent);

private:
	UPROPERTY()	FGameplayAttributeData Health;
	UPROPERTY()	FGameplayAttributeData MaxHealth;
	UPROPERTY()	FGameplayAttributeData PhysicalDamage;
	UPROPERTY()	FGameplayAttributeData PhysicalPenetration;
	UPROPERTY()	FGameplayAttributeData MagicalDamage;
	UPROPERTY()	FGameplayAttributeData MagicalPenetration;
	UPROPERTY()	FGameplayAttributeData PhysicalArmor;
	UPROPERTY()	FGameplayAttributeData MagicalArmor;
	UPROPERTY()	FGameplayAttributeData CriticalChance;
	UPROPERTY()	FGameplayAttributeData CriticalDamage;
	UPROPERTY()	FGameplayAttributeData AttackSpeed;
	UPROPERTY()	FGameplayAttributeData MoveSpeed;
	UPROPERTY()	FGameplayAttributeData SkillCooldownSpeed;
	UPROPERTY()	FGameplayAttributeData WeaponSwapSpeed;
	UPROPERTY()	FGameplayAttributeData CurrentDodgeCount;
	UPROPERTY()	FGameplayAttributeData MaxDodgeCount;
	
	UPROPERTY()	FGameplayAttributeData CachedHealthPercent;
};
