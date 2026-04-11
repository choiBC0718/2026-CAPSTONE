// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CAP_AttributeTabWidget.generated.h"

/**
 * 캐릭터의 스탯을 나열해 보여줄 위젯 (CAP_AttributeSlotWidget) 나열
 */
UCLASS()
class UCAP_AttributeTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:

private:
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* PhysicalDamage;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* PhysicalPenetration;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* MagicalDamage;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* MagicalPenetration;

	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* PhysicalArmor;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* MagicalArmor;
	
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* CriticalChance;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* CriticalDamage;

	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* AttackSpeed;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* MoveSpeed;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* SkillCooldownSpeed;
	UPROPERTY(meta = (BindWidget))
	class UCAP_AttributeSlotWidget* WeaponSwapSpeed;
};
