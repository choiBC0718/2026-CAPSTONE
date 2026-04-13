// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CAP_MenuInterface.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMenuClosedSignature);

UINTERFACE(MinimalAPI)
class UCAP_MenuInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ICAP_MenuInterface
{
	GENERATED_BODY()

public:
	virtual void NativeOpenMenu() =0;
	virtual void NativeCloseMenu() =0;
	virtual FOnMenuClosedSignature& GetOnMenuClosedDelegate()=0;
};
