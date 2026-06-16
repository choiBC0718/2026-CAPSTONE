// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"
#include "ShopOfferSet.generated.h"

class UCAP_WeaponDataAsset;

UENUM(BlueprintType)
enum class EShopOfferType : uint8
{
	RandomWeapon UMETA(DisplayName="Random Weapon"),
	MaxHealth UMETA(DisplayName="Max Health")
};

USTRUCT(BlueprintType)
struct FShopOfferBaseEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	FName OfferId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop", meta=(ClampMin="0.0"))
	float Weight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	ECurrencyType PriceCurrencyType = ECurrencyType::Gold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop", meta=(ClampMin="0"))
	int32 PriceAmount = 100;
};

USTRUCT(BlueprintType)
struct FShopWeaponOfferEntry : public FShopOfferBaseEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Weapon")
	TObjectPtr<UCAP_WeaponDataAsset> WeaponData;
};

USTRUCT(BlueprintType)
struct FShopMaxHealthOfferEntry : public FShopOfferBaseEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Max Health", meta=(ClampMin="1.0"))
	float MaxHealthBonus = 25.f;
};

USTRUCT()
struct FShopOfferEntry
{
	GENERATED_BODY()

	FName OfferId = NAME_None;
	EShopOfferType OfferType = EShopOfferType::RandomWeapon;
	float Weight = 1.f;
	ECurrencyType PriceCurrencyType = ECurrencyType::Gold;
	int32 PriceAmount = 100;
	TObjectPtr<UCAP_WeaponDataAsset> WeaponData;
	float MaxHealthBonus = 0.f;
};

UCLASS(BlueprintType)
class UShopOfferSet : public UDataAsset
{
	GENERATED_BODY()

public:
	bool PickOffer(const TArray<EShopOfferType>& AllowedOfferTypes, FRandomStream& RandomStream, FShopOfferEntry& OutOffer) const;
	bool PickOffer(const TArray<EShopOfferType>& AllowedOfferTypes, const TSet<FName>& ExcludedOfferIds, FRandomStream& RandomStream, FShopOfferEntry& OutOffer) const;
	bool FindOfferById(FName OfferId, FShopOfferEntry& OutOffer) const;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Weapon", meta=(AllowPrivateAccess="true", TitleProperty="OfferId"))
	TArray<FShopWeaponOfferEntry> WeaponOffers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop|Max Health", meta=(AllowPrivateAccess="true", TitleProperty="OfferId"))
	TArray<FShopMaxHealthOfferEntry> MaxHealthOffers;
};
