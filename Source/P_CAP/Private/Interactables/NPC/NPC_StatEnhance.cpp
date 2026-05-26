// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/NPC/NPC_StatEnhance.h"

ENPCActionResult ANPC_StatEnhance::ExecuteSpecialAction(AActor* Actor)
{
	return ENPCActionResult::OpenCustomWidget;
}
