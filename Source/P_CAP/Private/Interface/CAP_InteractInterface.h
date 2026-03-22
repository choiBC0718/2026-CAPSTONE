// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CAP_InteractInterface.generated.h"

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
	virtual void Interact(class ACAP_PlayerCharacter* PlayerCharacter) =0;
	virtual void ShowInterfaceWidget() {};
	virtual void HideInterfaceWidget() {};
};
