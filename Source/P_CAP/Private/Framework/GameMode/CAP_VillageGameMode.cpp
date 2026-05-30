// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameMode/CAP_VillageGameMode.h"

#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Kismet/GameplayStatics.h"

void ACAP_VillageGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (UGameInstance* GI=UGameplayStatics::GetGameInstance(this))
	{
		if (UCAP_ProgressionSubsystem* Subsys = GI->GetSubsystem<UCAP_ProgressionSubsystem>())
			Subsys->ClearRunStats();
	}
}
