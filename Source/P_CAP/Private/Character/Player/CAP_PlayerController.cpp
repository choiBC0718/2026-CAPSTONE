// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerController.h"

#include "CAP_PlayerCharacter.h"

void ACAP_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	PlayerCharacter = Cast<ACAP_PlayerCharacter>(InPawn);
	if (PlayerCharacter)
	{
		//메인 위젯 스폰 메소드
	}
}
