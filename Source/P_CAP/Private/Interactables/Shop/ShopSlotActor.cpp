// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Shop/ShopSlotActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Component/CAP_CurrencyComponent.h"
#include "Component/CAP_WeaponComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "Framework/Subsystem/CAP_ProgressionSubsystem.h"
#include "Interactables/Shop/ShopPriceWidget.h"
#include "Interactables/Weapon/CAP_WorldWeapon.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"
#include "Map/SpecialRoomTransitionSubsystem.h"

AShopSlotActor::AShopSlotActor()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemPedestalMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemPedestalMesh"));
	ItemPedestalMeshComponent->SetupAttachment(RootComponent);
	ItemPedestalMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	ItemPedestalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PreviewSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewSpawnPoint"));
	PreviewSpawnPoint->SetupAttachment(RootComponent);
	PreviewSpawnPoint->SetRelativeLocation(FVector(0.f, 0.f, 80.f));

	MaxHealthPreviewMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MaxHealthPreviewMesh"));
	MaxHealthPreviewMeshComponent->SetupAttachment(PreviewSpawnPoint);
	MaxHealthPreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MaxHealthPreviewMeshComponent->SetVisibility(false);

	PriceWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PriceWidget"));
	PriceWidgetComponent->SetupAttachment(RootComponent);
	PriceWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	PriceWidgetComponent->SetDrawSize(FVector2D(220.f, 80.f));
	PriceWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
}

void AShopSlotActor::BeginPlay()
{
	Super::BeginPlay();

	bPurchased = IsAlreadyPurchased();
	SelectOrRestoreOffer();
	RefreshPriceWidget();
	RefreshPreview();

	if (bPurchased && bDestroyWhenPurchased)
	{
		ClearPreview();
		Destroy();
	}
}

void AShopSlotActor::Interact(AActor* InsActor, EInteractAction ActionType)
{
	if (bPurchased || ActionType != EInteractAction::Tap)
	{
		return;
	}

	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor);
	if (!Player || !bHasSelectedOffer)
	{
		return;
	}

	if (TryBuy(*Player))
	{
		bPurchased = true;
		MarkPurchased();
		Player->SaveProgressionBeforeChangeLevel();
		RefreshPriceWidget();
		ClearPreview();

		if (bDestroyWhenPurchased)
		{
			Destroy();
		}
	}
}

FInteractionPayload AShopSlotActor::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	if (!bHasSelectedOffer)
	{
		Payload.ActionData.ShortActionText = NSLOCTEXT("Shop", "NoOffer", "No Stock").ToString();
		return Payload;
	}

	Payload.ActionData.bShowCurrency = true;
	Payload.ActionData.ActionCurrencyType = SelectedOffer.PriceCurrencyType;
	Payload.ActionData.CurrencyAmount = SelectedOffer.PriceAmount;
	Payload.ActionData.ShortActionText = bPurchased
		? NSLOCTEXT("Shop", "SoldOut", "Sold Out").ToString()
		: FText::Format(NSLOCTEXT("Shop", "BuyOfferFormat", "Buy {0}"), GetSelectedOfferDisplayName()).ToString();
	return Payload;
}

FString AShopSlotActor::BuildSpecialRoomShopKeyPrefix() const
{
	FIntPoint SpecialRoomGridPos = FIntPoint::ZeroValue;
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (const USpecialRoomTransitionSubsystem* TransitionSubsystem = GameInstance->GetSubsystem<USpecialRoomTransitionSubsystem>())
		{
			TransitionSubsystem->TryGetPendingSpecialRoomGridPos(SpecialRoomGridPos);
		}
	}

	return FString::Printf(TEXT("Shop_%d_%d_"), SpecialRoomGridPos.X, SpecialRoomGridPos.Y);
}

FName AShopSlotActor::BuildPersistentKey() const
{
	return FName(*FString::Printf(
		TEXT("%s%s"),
		*BuildSpecialRoomShopKeyPrefix(),
		*SlotId.ToString()));
}

void AShopSlotActor::SelectOrRestoreOffer()
{
	bHasSelectedOffer = false;
	SelectedOfferId = NAME_None;

	if (!OfferSet)
	{
		return;
	}

	const FName PersistentKey = BuildPersistentKey();
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USpecialRoomTransitionSubsystem* TransitionSubsystem = GameInstance->GetSubsystem<USpecialRoomTransitionSubsystem>())
		{
			FName SavedOfferId = NAME_None;
			if (TransitionSubsystem->TryGetSpecialRoomShopSlotOfferId(PersistentKey, SavedOfferId))
			{
				FShopOfferEntry RestoredOffer;
				if (OfferSet->FindOfferById(SavedOfferId, RestoredOffer))
				{
					SelectedOffer = RestoredOffer;
					SelectedOfferId = SavedOfferId;
					bHasSelectedOffer = true;
					return;
				}
			}

			int32 Seed = GetTypeHash(PersistentKey);
			Seed = HashCombineFast(Seed, 0x4F17A1C3);
			FRandomStream RandomStream(Seed);
			TSet<FName> ExcludedOfferIds;
			TransitionSubsystem->GetSpecialRoomShopOfferIdsByKeyPrefix(BuildSpecialRoomShopKeyPrefix(), ExcludedOfferIds);

			FShopOfferEntry PickedOffer;
			if (OfferSet->PickOffer(AllowedOfferTypes, ExcludedOfferIds, RandomStream, PickedOffer))
			{
				SelectedOffer = PickedOffer;
				SelectedOfferId = PickedOffer.OfferId;
				bHasSelectedOffer = true;
				TransitionSubsystem->SetSpecialRoomShopSlotOfferId(PersistentKey, SelectedOfferId);
			}
		}
	}
}

bool AShopSlotActor::TryBuy(ACAP_PlayerCharacter& Player)
{
	if (!bHasSelectedOffer)
	{
		return false;
	}

	UCAP_CurrencyComponent* CurrencyComponent = Player.GetCurrencyComponent();
	if (!CurrencyComponent || !CurrencyComponent->ConsumeCurrency(SelectedOffer.PriceCurrencyType, SelectedOffer.PriceAmount))
	{
		return false;
	}

	bool bGranted = false;
	switch (SelectedOffer.OfferType)
	{
	case EShopOfferType::RandomWeapon:
		bGranted = GiveWeaponOffer(Player, SelectedOffer);
		break;

	case EShopOfferType::MaxHealth:
		bGranted = GiveMaxHealthOffer(Player, SelectedOffer);
		break;

	default:
		break;
	}

	if (!bGranted)
	{
		CurrencyComponent->AddCurrency(SelectedOffer.PriceCurrencyType, SelectedOffer.PriceAmount);
	}

	return bGranted;
}

bool AShopSlotActor::GiveWeaponOffer(ACAP_PlayerCharacter& Player, const FShopOfferEntry& Offer)
{
	if (!Offer.WeaponData)
	{
		return false;
	}

	UCAP_WeaponComponent* WeaponComponent = Player.GetWeaponComponent();
	if (!WeaponComponent)
	{
		return false;
	}

	UCAP_WeaponInstance* NewWeaponInstance = NewObject<UCAP_WeaponInstance>(this);
	if (!NewWeaponInstance)
	{
		return false;
	}

	NewWeaponInstance->Initialize(Offer.WeaponData);
	WeaponComponent->PickupWeapon(NewWeaponInstance);
	return true;
}

bool AShopSlotActor::GiveMaxHealthOffer(ACAP_PlayerCharacter& Player, const FShopOfferEntry& Offer) const
{
	if (Offer.MaxHealthBonus <= 0.f)
	{
		return false;
	}

	UCAP_AbilitySystemComponent* AbilitySystemComponent = Cast<UCAP_AbilitySystemComponent>(
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(&Player));
	if (!AbilitySystemComponent)
	{
		return false;
	}

	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UCAP_AttributeSet::GetMaxHealthAttribute(),
		EGameplayModOp::Additive,
		Offer.MaxHealthBonus);
	AbilitySystemComponent->ApplyModToAttributeUnsafe(
		UCAP_AttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		Offer.MaxHealthBonus);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UCAP_ProgressionSubsystem* ProgressionSubsystem = GameInstance->GetSubsystem<UCAP_ProgressionSubsystem>())
		{
			ProgressionSubsystem->AddBonusMaxHealth(Offer.MaxHealthBonus);
		}
	}

	return true;
}

bool AShopSlotActor::IsAlreadyPurchased() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (const USpecialRoomTransitionSubsystem* TransitionSubsystem = GameInstance->GetSubsystem<USpecialRoomTransitionSubsystem>())
		{
			return TransitionSubsystem->IsSpecialRoomShopSlotPurchased(BuildPersistentKey());
		}
	}

	return false;
}

void AShopSlotActor::MarkPurchased()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (USpecialRoomTransitionSubsystem* TransitionSubsystem = GameInstance->GetSubsystem<USpecialRoomTransitionSubsystem>())
		{
			TransitionSubsystem->MarkSpecialRoomShopSlotPurchased(BuildPersistentKey());
		}
	}
}

void AShopSlotActor::RefreshPriceWidget()
{
	if (!PriceWidgetComponent)
	{
		return;
	}

	UShopPriceWidget* PriceWidget = Cast<UShopPriceWidget>(PriceWidgetComponent->GetWidget());
	if (!PriceWidget || !bHasSelectedOffer)
	{
		return;
	}

	PriceWidget->SetOfferDisplayData(
		SelectedOffer.PriceCurrencyType,
		SelectedOffer.PriceAmount,
		bPurchased);
}

void AShopSlotActor::RefreshPreview()
{
	ClearPreview();

	if (!bHasSelectedOffer || bPurchased)
	{
		return;
	}

	if (SelectedOffer.OfferType == EShopOfferType::RandomWeapon && SelectedOffer.WeaponData)
	{
		TSubclassOf<ACAP_WorldWeapon> PreviewClass = WeaponPreviewClass;
		if (!PreviewClass)
		{
			PreviewClass = ACAP_WorldWeapon::StaticClass();
		}

		const FTransform PreviewTransform = WeaponPreviewRelativeTransform * PreviewSpawnPoint->GetComponentTransform();
		ACAP_WorldWeapon* WorldWeapon = GetWorld()->SpawnActorDeferred<ACAP_WorldWeapon>(
			PreviewClass,
			PreviewTransform,
			this,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (WorldWeapon)
		{
			WorldWeapon->InitializeWeaponData(SelectedOffer.WeaponData);
			WorldWeapon->FinishSpawning(PreviewTransform);
			DisablePreviewActorInteraction(*WorldWeapon);
			WorldWeapon->AttachToComponent(PreviewSpawnPoint, FAttachmentTransformRules::KeepWorldTransform);
			PreviewActor = WorldWeapon;
		}
		return;
	}

	if (SelectedOffer.OfferType == EShopOfferType::MaxHealth && MaxHealthPreviewMesh)
	{
		MaxHealthPreviewMeshComponent->SetStaticMesh(MaxHealthPreviewMesh);
		MaxHealthPreviewMeshComponent->SetVisibility(true, true);
	}
}

void AShopSlotActor::ClearPreview()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}

	if (MaxHealthPreviewMeshComponent)
	{
		MaxHealthPreviewMeshComponent->SetVisibility(false, true);
	}
}

void AShopSlotActor::DisablePreviewActorInteraction(AActor& Actor) const
{
	Actor.SetActorEnableCollision(false);

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Actor.GetComponents(PrimitiveComponents);
	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetSimulatePhysics(false);
		PrimitiveComponent->SetEnableGravity(false);
		PrimitiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PrimitiveComponent->SetGenerateOverlapEvents(false);
	}
}

FText AShopSlotActor::GetSelectedOfferDisplayName() const
{
	if (!bHasSelectedOffer)
	{
		return FText::GetEmpty();
	}

	if (SelectedOffer.OfferType == EShopOfferType::RandomWeapon && SelectedOffer.WeaponData)
	{
		return SelectedOffer.WeaponData->ItemName;
	}

	if (SelectedOffer.OfferType == EShopOfferType::MaxHealth)
	{
		return FText::Format(
			NSLOCTEXT("Shop", "MaxHealthDisplayName", "Max HP +{0}"),
			FText::AsNumber(FMath::RoundToInt(SelectedOffer.MaxHealthBonus)));
	}

	return FText::FromName(SelectedOffer.OfferId);
}
