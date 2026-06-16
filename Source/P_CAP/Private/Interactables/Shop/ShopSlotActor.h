// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactables/CAP_InteractableBase.h"
#include "ShopOfferSet.h"
#include "ShopSlotActor.generated.h"

class ACAP_PlayerCharacter;
class ACAP_WorldWeapon;
class UStaticMesh;
class UStaticMeshComponent;
class UShopPriceWidget;
class UWidgetComponent;

UCLASS(Blueprintable)
class AShopSlotActor : public ACAP_InteractableBase
{
	GENERATED_BODY()

public:
	AShopSlotActor();

	virtual void BeginPlay() override;
	virtual void Interact(AActor* InsActor, EInteractAction ActionType) override;
	virtual FInteractionPayload GetInteractionPayload() const override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shop|Preview")
	TObjectPtr<UStaticMeshComponent> ItemPedestalMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shop|Preview")
	TObjectPtr<USceneComponent> PreviewSpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shop|Preview")
	TObjectPtr<UStaticMeshComponent> MaxHealthPreviewMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shop|UI")
	TObjectPtr<UWidgetComponent> PriceWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	FName SlotId = TEXT("Slot_0");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	TObjectPtr<UShopOfferSet> OfferSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	TArray<EShopOfferType> AllowedOfferTypes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	bool bDestroyWhenPurchased = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Preview")
	TSubclassOf<ACAP_WorldWeapon> WeaponPreviewClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Preview")
	TObjectPtr<UStaticMesh> MaxHealthPreviewMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Preview")
	FTransform WeaponPreviewRelativeTransform = FTransform(FRotator::ZeroRotator, FVector(0.f, 0.f, 80.f), FVector(1.f));

private:
	bool bPurchased = false;
	bool bHasSelectedOffer = false;
	FName SelectedOfferId = NAME_None;
	FShopOfferEntry SelectedOffer;
	UPROPERTY()
	TObjectPtr<AActor> PreviewActor;

	FString BuildSpecialRoomShopKeyPrefix() const;
	FName BuildPersistentKey() const;
	void SelectOrRestoreOffer();
	bool TryBuy(ACAP_PlayerCharacter& Player);
	bool GiveWeaponOffer(ACAP_PlayerCharacter& Player, const FShopOfferEntry& Offer);
	bool GiveMaxHealthOffer(ACAP_PlayerCharacter& Player, const FShopOfferEntry& Offer) const;
	bool IsAlreadyPurchased() const;
	void MarkPurchased();
	void RefreshPriceWidget();
	void RefreshPreview();
	void ClearPreview();
	void DisablePreviewActorInteraction(AActor& Actor) const;
	FText GetSelectedOfferDisplayName() const;
};
