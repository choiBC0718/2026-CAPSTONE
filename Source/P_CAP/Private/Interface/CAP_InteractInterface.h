// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "UObject/Interface.h"
#include "CAP_InteractInterface.generated.h"


UENUM(BlueprintType)
enum class ENPCActionResult : uint8
{
	Success					UMETA(DisplayName = "Success (특수 로직 성공)"),
	InsufficientCurrency	UMETA(DisplayName = "InsufficientCurrency (재화 부족)"),
	AlreadyReceived			UMETA(DisplayName = "AlreadyReceived (특수 로직 기회 없음)"),
	Failed					UMETA(DisplayName = "Failed (실패)"),
	RequireConfirm			UMETA(DisplayName = "RequireConfirm (비용 확인 대사)"),
};

USTRUCT(BlueprintType)
struct FNPCData
{
	GENERATED_BODY()
	// NPC 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FString NPCName;
	// NPC 이미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	class UTexture2D* NPCImage = nullptr;
	// 해당 NPC와 대화 시 나타날 첫 대사
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(MultiLine=true))
	FString DefaultDialogue;
	// NPC 특수 행동 버튼에 들어갈 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FString SpecialActionText;

	// 특수 버튼 선택시 상황에 맞는 대사 -> 같은 ActionResult에 다양항 대사 가능하도록 추가 예정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(MultiLine=true))
	TMap<ENPCActionResult, FString> ResultDialogues;
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
	virtual void Interact(AActor* InsActor, EInteractAction ActionType) = 0;
	virtual FInteractionPayload GetInteractionPayload() const = 0;

	// NPC와의 상호작용이 현재 어떤 상황인지
	virtual ENPCActionResult ExecuteSpecialAction(AActor* Actor) =0;
	// 실행 전 비용이 드는지 확인하는
	virtual bool GetSpecialActionCost(AActor* Actor, int32& OutCost) =0;
};
