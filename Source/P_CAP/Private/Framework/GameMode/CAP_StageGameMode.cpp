// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameMode/CAP_StageGameMode.h"
#include "Character/Player/CAP_PlayerController.h"
#include "Kismet/GameplayStatics.h"

void ACAP_StageGameMode::OnPlayerDeathAnimationFinished(class APlayerController* PC)
{
	if (!PC)
		return;
	
	GetWorldTimerManager().SetTimer(RagdollWaitTimerHandle,FTimerDelegate::CreateUObject(this, &ACAP_StageGameMode::ShowDeathDashboard, PC),RagdollSettleTime,false);
}

void ACAP_StageGameMode::ShowDeathDashboard(class APlayerController* PC)
{
	if (!PC)
		return;
	
	UGameplayStatics::SetGamePaused(GetWorld(), true);
	if (ACAP_PlayerController* CAP_PC = Cast<ACAP_PlayerController>(PC))
	{
		if (UCAP_GameplayWidget* HUD = CAP_PC->GetGameplayWidget())
		{
			// GameplayWidget에서 대시보드 위젯 활성화
		}
	}
}
