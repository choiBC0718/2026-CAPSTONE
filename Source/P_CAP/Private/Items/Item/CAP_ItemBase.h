// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CAP_ItemInstance.h"
#include "Items/CAP_DropItemBase.h"
#include "CAP_ItemBase.generated.h"

/**
 * 
 */
UCLASS()
class ACAP_ItemBase : public ACAP_DropItemBase
{
	GENERATED_BODY()

public:
	ACAP_ItemBase();
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

	virtual void InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter) override;
	virtual void InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter) override;
	virtual UObject* GetInteractData() const override {return ItemInstance;}

	UPROPERTY(EditDefaultsOnly, Category="Item Data")
	class UCAP_ItemDataAsset* ItemDA;

	UPROPERTY(BlueprintReadWrite, Category="Item Data", meta=(ExposeOnSpawn="true"))
	class UCAP_ItemInstance* ItemInstance;

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	class UStaticMeshComponent* ItemMesh;
};
