// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "UObject/Interface.h"
#include "CAP_InteractInterface.generated.h"


USTRUCT(BlueprintType)
struct FActionPromptData
{
	GENERATED_BODY()

	// 짧게 누르기 텍스트 (줍기, 구매하기, 대화하기 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	FString ShortActionText = "";
	// 길게 누르기 텍스트 (파괴하기 / 비어있으면 Hidden처리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	FString LongActionText = "";
	// 재화를 표시할지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	bool bShowCurrency=false;
	// 표시할 재화 종류
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	ECurrencyType ActionCurrencyType = ECurrencyType::Gold;
	// 재화 수치
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	int32 CurrencyAmount=0;
};

USTRUCT(BlueprintType)
struct FInteractionPayload
{
	GENERATED_BODY()

	// 상단 패널용 데이터 (아이템 정보, 무기 정보_ Instance / 없으면 숨김처리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	UObject* DetailData =nullptr;
	// 하단 패널 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	FActionPromptData ActionData;
};

UENUM(BlueprintType)
enum class EInteractAction : uint8
{
	Tap     UMETA(DisplayName = "Tap (짧게 누르기)"),
	Hold    UMETA(DisplayName = "Hold (길게 누르기)")
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCAP_InteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ICAP_InteractInterface
{
	GENERATED_BODY()

public:
	// 상호작용 키를 눌렀을 때 작동할 함수
	virtual void Interact(AActor* InsActor, EInteractAction ActionType) = 0;
	// 상호작용 할 대상에 어울리는 정보로 UI 갱신하기 위한 구조체 (텍스트, 객체)
	virtual FInteractionPayload GetInteractionPayload() const = 0;
};
