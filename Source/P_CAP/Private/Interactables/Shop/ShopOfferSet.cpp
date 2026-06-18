// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Shop/ShopOfferSet.h"

#include "Data/CAP_WeaponDataAsset.h"

namespace
{
	void CopyBaseFields(const FShopOfferBaseEntry& Source, FShopOfferEntry& Target)
	{
		Target.OfferId = Source.OfferId;
		Target.Weight = Source.Weight;
		Target.PriceCurrencyType = Source.PriceCurrencyType;
		Target.PriceAmount = Source.PriceAmount;
	}

	FShopOfferEntry MakeWeaponOffer(const FShopWeaponOfferEntry& Source)
	{
		FShopOfferEntry Offer;
		CopyBaseFields(Source, Offer);
		Offer.OfferType = EShopOfferType::RandomWeapon;
		Offer.WeaponData = Source.WeaponData;
		return Offer;
	}

	FShopOfferEntry MakeMaxHealthOffer(const FShopMaxHealthOfferEntry& Source)
	{
		FShopOfferEntry Offer;
		CopyBaseFields(Source, Offer);
		Offer.OfferType = EShopOfferType::MaxHealth;
		Offer.MaxHealthBonus = Source.MaxHealthBonus;
		return Offer;
	}

	bool IsOfferAllowed(const FShopOfferEntry& Offer, const TArray<EShopOfferType>& AllowedOfferTypes)
	{
		return AllowedOfferTypes.IsEmpty() || AllowedOfferTypes.Contains(Offer.OfferType);
	}

	bool IsOfferValid(const FShopOfferEntry& Offer)
	{
		if (Offer.OfferId.IsNone() || Offer.Weight <= 0.f)
		{
			return false;
		}

		switch (Offer.OfferType)
		{
		case EShopOfferType::RandomWeapon:
			return Offer.WeaponData != nullptr;

		case EShopOfferType::MaxHealth:
			return Offer.MaxHealthBonus > 0.f;

		default:
			return false;
		}
	}
}

bool UShopOfferSet::PickOffer(const TArray<EShopOfferType>& AllowedOfferTypes, FRandomStream& RandomStream, FShopOfferEntry& OutOffer) const
{
	return PickOffer(AllowedOfferTypes, TSet<FName>(), RandomStream, OutOffer);
}

bool UShopOfferSet::PickOffer(const TArray<EShopOfferType>& AllowedOfferTypes, const TSet<FName>& ExcludedOfferIds, FRandomStream& RandomStream, FShopOfferEntry& OutOffer) const
{
	TArray<FShopOfferEntry> CandidateOffers;
	CandidateOffers.Reserve(WeaponOffers.Num() + MaxHealthOffers.Num());

	for (const FShopWeaponOfferEntry& WeaponOffer : WeaponOffers)
	{
		CandidateOffers.Add(MakeWeaponOffer(WeaponOffer));
	}

	for (const FShopMaxHealthOfferEntry& MaxHealthOffer : MaxHealthOffers)
	{
		CandidateOffers.Add(MakeMaxHealthOffer(MaxHealthOffer));
	}

	float TotalWeight = 0.f;
	for (const FShopOfferEntry& Offer : CandidateOffers)
	{
		if (!ExcludedOfferIds.Contains(Offer.OfferId) && IsOfferAllowed(Offer, AllowedOfferTypes) && IsOfferValid(Offer))
		{
			TotalWeight += Offer.Weight;
		}
	}

	if (TotalWeight <= 0.f)
	{
		return false;
	}

	const float RandomPick = RandomStream.FRandRange(0.f, TotalWeight);
	float RunningWeight = 0.f;
	for (const FShopOfferEntry& Offer : CandidateOffers)
	{
		if (ExcludedOfferIds.Contains(Offer.OfferId) || !IsOfferAllowed(Offer, AllowedOfferTypes) || !IsOfferValid(Offer))
		{
			continue;
		}

		RunningWeight += Offer.Weight;
		if (RandomPick <= RunningWeight)
		{
			OutOffer = Offer;
			return true;
		}
	}

	return false;
}

bool UShopOfferSet::FindOfferById(FName OfferId, FShopOfferEntry& OutOffer) const
{
	if (OfferId.IsNone())
	{
		return false;
	}

	for (const FShopWeaponOfferEntry& WeaponOffer : WeaponOffers)
	{
		FShopOfferEntry Offer = MakeWeaponOffer(WeaponOffer);
		if (Offer.OfferId == OfferId && IsOfferValid(Offer))
		{
			OutOffer = Offer;
			return true;
		}
	}

	for (const FShopMaxHealthOfferEntry& MaxHealthOffer : MaxHealthOffers)
	{
		FShopOfferEntry Offer = MakeMaxHealthOffer(MaxHealthOffer);
		if (Offer.OfferId == OfferId && IsOfferValid(Offer))
		{
			OutOffer = Offer;
			return true;
		}
	}

	return false;
}
