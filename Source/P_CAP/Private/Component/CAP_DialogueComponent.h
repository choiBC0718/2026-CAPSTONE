// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CAP_DialogueComponent.generated.h"

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
	// 대화하기 버튼으로 나타낼 대사
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(MultiLine=true))
	FString SmallTalkText;
	// NPC 특수 행동 버튼에 들어갈 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FString SpecialActionText;

	// 특수 버튼 선택시 상황에 맞는 대사 -> 같은 ActionResult에 다양항 대사 가능하도록 추가 예정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue", meta=(MultiLine=true))
	TMap<ENPCActionResult, FString> ResultDialogues;
};

DECLARE_DELEGATE_RetVal_OneParam(ENPCActionResult, FOnExecuteSpecialAction, AActor* /*Interactor*/);
DECLARE_DELEGATE_RetVal_OneParam(bool, FOnGetSpecialActionCost, int32& /*OutCost*/);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UCAP_DialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCAP_DialogueComponent();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BeginDialogue(class ACAP_PlayerCharacter* InPlayer);
	void EndDialogue();

	UPROPERTY(EditAnywhere, Category="Dialogue")
	FNPCData NPCData;

	FOnExecuteSpecialAction OnExecuteSpecialAction;
	FOnGetSpecialActionCost OnGetSpecialActionCost;

	ENPCActionResult ExecuteSpecialAction(AActor* InteractActor);
	bool GetSpecialActionCost(int32& OutCost);

private:
	UPROPERTY()
	class ACAP_PlayerCharacter* Player;

	float OriginArmLength;
	FRotator OriginArmRotation;
	FVector OriginSocketOffset;

	float TargetArmLength;
	FRotator TargetArmRotation;
	FVector TargetSocketOffset;

	// 카메라 움직일 때만 Tick이 돌아가도록 하는 bool
	bool bIsCameraMoving = false;
	void UpdateCamera(float DeltaTime);
};
