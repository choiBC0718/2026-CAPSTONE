// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "CAP_WeaponSkillBox.generated.h"

/**
 * 장착한 무기에 부여된 스킬의 아이콘을 AbilitySlot에 적용
 * 무기 이름, 설명, 아이콘의 데이터를 담아 설정
 */
UCLASS()
class UCAP_WeaponSkillBox : public UUserWidget
{
	GENERATED_BODY()

public:
	// 무기 데이터 새로고침 (무기 변경 시 호출)
	void RefreshWeaponSkills(class UCAP_WeaponInstance* MainWeaponInst, class UCAP_WeaponInstance* SubWeaponInst);

private:
	// 부여된 스킬 아이콘 WrapBox
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* MainAbilityWrapBox;
	UPROPERTY(meta = (BindWidget))
	class UWrapBox* SubAbilityWrapBox;
	
	UPROPERTY(EditDefaultsOnly, Category="Ability")
	TSubclassOf<class UCAP_AbilitySlot> AbilitySlotClass;
};
