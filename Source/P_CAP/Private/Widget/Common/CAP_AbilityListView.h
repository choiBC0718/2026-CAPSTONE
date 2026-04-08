// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "CAP_AbilityListView.generated.h"

/**
 * 장착한 무기에 부여된 스킬의 아이콘을 AbilitySlot에 적용
 * 무기 이름, 설명, 아이콘의 데이터를 담아 설정
 */
UCLASS()
class UCAP_AbilityListView : public UListView
{
	GENERATED_BODY()

public:
	// 무기 데이터 새로고침 (무기 변경 시 호출)
	void RefreshWeaponSkills(class UCAP_WeaponInstance* WeaponInstance);
};
