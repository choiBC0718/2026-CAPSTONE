// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/Payload/GA_Payload_ApplyBuff.h"

#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"

struct FModifierGroup
{
	bool bIsMultiplier;
	float Duration;
	TMap<FGameplayTag, float> StatMods;
};

FBuffDisplayData UGA_Payload_ApplyBuff::GetBuffDisplayData(const FGameplayTag& EffectTag) const
{
	FBuffDisplayData BuffData;
	if (const FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		if (UCAP_WeaponInstance* WeaponInst = Cast<UCAP_WeaponInstance>(Spec->SourceObject))
		{
			for (const FWeaponSkillData& SkillData : WeaponInst->GetGrantedSkills())
			{
				if (SkillData.PayloadAbilityClass.Contains(GetClass()))
				{
					BuffData.Icon = SkillData.SkillIcon;
					return BuffData;
				}
			}
			
			if (const FWeaponSkillData* BasicAttack = WeaponInst->GetBasicAttack())
			{
				if (BasicAttack->PayloadAbilityClass.Contains(GetClass()))
				{
					BuffData.Icon = BasicAttack->SkillIcon;
					return BuffData;
				}
			}
		}
	}
	return BuffData;
}

void UGA_Payload_ApplyBuff::ExecutePayloadLogic(const FGameplayEventData& EventData)
{
	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
	if (!ASC || !ASC->GetGenerics())
		return;
	
	TArray<FModifierGroup> ModifierGroups;
	for (const FSkillStatModifier& Mod : SkillStatModifiers)
	{
		bool bFoundGroup = false;
		for (FModifierGroup& Group : ModifierGroups)
		{
			if (Group.bIsMultiplier == Mod.IsMultiplier && FMath::IsNearlyEqual(Group.Duration, Mod.Duration))
			{
				Group.StatMods.Add(Mod.StatTag,Mod.Value);
				bFoundGroup = true;
				break;
			}
		}
		if (!bFoundGroup)
		{
			FModifierGroup NewGroup;
			NewGroup.bIsMultiplier = Mod.IsMultiplier;
			NewGroup.Duration = Mod.Duration;
			NewGroup.StatMods.Add(Mod.StatTag,Mod.Value);
			ModifierGroups.Add(NewGroup);
		}
	}

	static FGameplayTag DurationTag = UCAP_AbilitySystemStatics::GetDataEffectDurationTag();
	
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);
	for (const FModifierGroup& Group : ModifierGroups)
	{
		TSubclassOf<UGameplayEffect> TargetGE = ASC->GetGenerics()->GetStatGE(false,Group.bIsMultiplier);
		if (!TargetGE)
			continue;
		
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(TargetGE, 1.f, Context);
		if (SpecHandle.IsValid())
		{
			SpecHandle.Data->SetSetByCallerMagnitude(DurationTag ,Group.Duration);
			SpecHandle.Data->DynamicGrantedTags.AddTag(FGameplayTag::RequestGameplayTag("UI.Buff"));
		
			if (const UGameplayEffect* DefaultGE = TargetGE->GetDefaultObject<UGameplayEffect>())
			{
				for (const FGameplayModifierInfo& ModInfo : DefaultGE->Modifiers)
				{
					if (ModInfo.ModifierMagnitude.GetMagnitudeCalculationType() == EGameplayEffectMagnitudeCalculation::SetByCaller)
					{
						FGameplayTag CallerTag = ModInfo.ModifierMagnitude.GetSetByCallerFloat().DataTag;
						SpecHandle.Data->SetSetByCallerMagnitude(CallerTag, Group.bIsMultiplier ? 1.f : 0.f);
					}
				}
			}

			for (const TPair<FGameplayTag, float>& StatMod : Group.StatMods)
			{
				SpecHandle.Data->SetSetByCallerMagnitude(StatMod.Key, StatMod.Value);
			}
			ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());	
		}
	}
}
