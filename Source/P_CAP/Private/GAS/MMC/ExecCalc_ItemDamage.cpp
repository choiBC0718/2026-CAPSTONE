// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC/ExecCalc_ItemDamage.h"

#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

UExecCalc_ItemDamage::UExecCalc_ItemDamage()
{
	PhysicalArmorCapture.AttributeToCapture = UCAP_AttributeSet::GetPhysicalArmorAttribute();
	PhysicalArmorCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	MagicalArmorCapture.AttributeToCapture = UCAP_AttributeSet::GetMagicalArmorAttribute();
	MagicalArmorCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	PhysicalPenCapture.AttributeToCapture = UCAP_AttributeSet::GetPhysicalPenetrationAttribute();
	PhysicalPenCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	MagicalPenCapture.AttributeToCapture = UCAP_AttributeSet::GetMagicalPenetrationAttribute();
	MagicalPenCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	CriticalChanceCapture.AttributeToCapture = UCAP_AttributeSet::GetCriticalChanceAttribute();
	CriticalChanceCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	CriticalDamageCapture.AttributeToCapture = UCAP_AttributeSet::GetCriticalDamageAttribute();
	CriticalDamageCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	RelevantAttributesToCapture.Add(PhysicalArmorCapture);
	RelevantAttributesToCapture.Add(MagicalArmorCapture);
	RelevantAttributesToCapture.Add(PhysicalPenCapture);
	RelevantAttributesToCapture.Add(MagicalPenCapture);
	RelevantAttributesToCapture.Add(CriticalChanceCapture);
	RelevantAttributesToCapture.Add(CriticalDamageCapture);
}

void UExecCalc_ItemDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FGameplayTag BaseDamageTag = UCAP_AbilitySystemStatics::GetDataDamageBaseTag();
	float FinalDamage = Spec.GetSetByCallerMagnitude(BaseDamageTag, false, 0.f);

	bool bIsPhysical = Spec.DynamicGrantedTags.HasTag(FGameplayTag::RequestGameplayTag("Damage.Type.Physical"));
	bool bIsMagical = Spec.DynamicGrantedTags.HasTag(FGameplayTag::RequestGameplayTag("Damage.Type.Magical"));

	float Armor = 0.f;
	float Pen = 0.f;
	
	if (bIsPhysical)
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalArmorCapture, EvalParams, Armor);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalPenCapture, EvalParams, Pen);
	}
	else if (bIsMagical)
	{
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicalArmorCapture, EvalParams, Armor);
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicalPenCapture, EvalParams, Pen);
	}

	if (bIsPhysical || bIsMagical)
	{
		float Diff = Pen - Armor;
		float DiffAmp = (Diff >= 0.0f) ? (1.0f + (Diff / 100.0f)) : (100.0f / (100.0f - Diff));
		FinalDamage *= DiffAmp;
	}

	float CriticalChance = 0.f;
	float CriticalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalChanceCapture, EvalParams, CriticalChance);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalDamageCapture, EvalParams, CriticalDamage);
	CriticalDamage = FMath::Max<float>(0.f, CriticalDamage);

	bool bCriticalHit = false;
	if (CriticalChance > 0.0f && FMath::RandRange(0.0f, 100.0f) <= CriticalChance)
	{
		FinalDamage *= CriticalDamage;
		bCriticalHit = true;
	}

	if (FCAP_GameplayEffectContext* CContext = static_cast<FCAP_GameplayEffectContext*>(Spec.GetContext().Get()))
	{
		CContext->bIsCritical = bCriticalHit;
	}

	FinalDamage = FMath::RoundToInt(FinalDamage);
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UCAP_AttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
}
