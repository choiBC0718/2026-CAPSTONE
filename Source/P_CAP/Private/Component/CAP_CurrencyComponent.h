// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "CAP_CurrencyComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCurrencyChanged, ECurrencyType, CurrencyType, int32, OldAmount, int32, NewAmount);

/*
 *	재화 관리 컴포넌트
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCAP_CurrencyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCAP_CurrencyComponent();
	virtual void BeginPlay() override;

	// 현재 재화 타입에 Amount만큼 더해줌
	UFUNCTION()
	void AddCurrency(ECurrencyType Type, int32 Amount);
	// 재화 소모량 Amount 만큼 소모
	UFUNCTION()
	bool ConsumeCurrency(ECurrencyType Type, int32 Amount);
	UFUNCTION()
	int32 GetCurreny(ECurrencyType Type);

	// 마석 (저장되는 재화)에 대해 강제 로딩
	void SetCurrencyOverride(ECurrencyType Type, int32 Amount);

	UPROPERTY()
	FOnCurrencyChanged OnCurrencyChanged;

private:
	UPROPERTY()
	TMap<ECurrencyType, int32> CurrencyMap;
};
