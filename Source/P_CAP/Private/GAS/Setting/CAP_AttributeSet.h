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
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, Health);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, PhysicalDamage);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MagicalDamage);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, PhysicalDefense);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MagicalDefense);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, MoveSpeed);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, CriticalChance);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, CriticalDamage);
	ATTRIBUTE_ACCESSORS(UCAP_AttributeSet, DodgeMaxCount);
	
private:
	UPROPERTY()	FGameplayAttributeData Health;
	UPROPERTY()	FGameplayAttributeData MaxHealth;
	UPROPERTY()	FGameplayAttributeData PhysicalDamage;
	UPROPERTY()	FGameplayAttributeData MagicalDamage;
	UPROPERTY()	FGameplayAttributeData PhysicalDefense;
	UPROPERTY()	FGameplayAttributeData MagicalDefense;
	UPROPERTY()	FGameplayAttributeData MoveSpeed;
	UPROPERTY()	FGameplayAttributeData CriticalChance;
	UPROPERTY()	FGameplayAttributeData CriticalDamage;
	UPROPERTY()	FGameplayAttributeData DodgeMaxCount;
	
};
