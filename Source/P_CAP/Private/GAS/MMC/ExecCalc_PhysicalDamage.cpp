// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC/ExecCalc_PhysicalDamage.h"

#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Setting/CAP_AttributeSet.h"

UExecCalc_PhysicalDamage::UExecCalc_PhysicalDamage()
{
	PhysicalDamageCapture.AttributeToCapture = UCAP_AttributeSet::GetPhysicalDamageAttribute();
	PhysicalDamageCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	PhysicalPenetrationCapture.AttributeToCapture = UCAP_AttributeSet::GetPhysicalPenetrationAttribute();
	PhysicalPenetrationCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	PhysicalArmorCapture.AttributeToCapture = UCAP_AttributeSet::GetPhysicalArmorAttribute();
	PhysicalArmorCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	CriticalChanceCapture.AttributeToCapture = UCAP_AttributeSet::GetCriticalChanceAttribute();
	CriticalChanceCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	CriticalDamageCapture.AttributeToCapture = UCAP_AttributeSet::GetCriticalDamageAttribute();
	CriticalDamageCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	RelevantAttributesToCapture.Add(PhysicalDamageCapture);
	RelevantAttributesToCapture.Add(PhysicalPenetrationCapture);
	RelevantAttributesToCapture.Add(PhysicalArmorCapture);
	RelevantAttributesToCapture.Add(CriticalChanceCapture);
	RelevantAttributesToCapture.Add(CriticalDamageCapture);

	DamageMultiplierDataTag = UCAP_AbilitySystemStatics::GetDataDamageMultiplierDataTag();
	ChargeMultiplierDataTag = UCAP_AbilitySystemStatics::GetAbilityChargeTimeTag();
}

void UExecCalc_PhysicalDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	float PhysicalDamage = 0.f;
	float PhysicalPen = 0.f;
	float PhysicalArmor = 0.f;
	float CriticalChance = 0.f;
	float CriticalDamage = 0.f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalDamageCapture, EvalParams, PhysicalDamage);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalPenetrationCapture, EvalParams, PhysicalPen);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(PhysicalArmorCapture, EvalParams, PhysicalArmor);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalChanceCapture, EvalParams, CriticalChance);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalDamageCapture, EvalParams, CriticalDamage);
	
	PhysicalDamage = FMath::Max<float>(0.f, PhysicalDamage);
	CriticalDamage = FMath::Max<float>(1.0f, CriticalDamage);

	// 기본 데미지 연산
	float Diff = PhysicalPen - PhysicalArmor;
	float DamageMultiplier = (Diff >= 0.0f) ? (1.0f + (Diff / 100.0f)) : (100.0f / (100.0f - Diff));
	float FinalDamage = PhysicalDamage * DamageMultiplier;

	// 스킬 계수 연산
	float SkillMultiplier = Spec.GetSetByCallerMagnitude(DamageMultiplierDataTag, false, 1.0f);
	FinalDamage *= SkillMultiplier;
	float ChargeMultiplier = Spec.GetSetByCallerMagnitude(ChargeMultiplierDataTag, false, 1.0f);
	FinalDamage *= ChargeMultiplier;
	
	bool bCriticalHit = false;
	if (CriticalChance > 0.0f)
	{
		if (FMath::RandRange(0.0f, 100.0f) <= CriticalChance)
		{
			bCriticalHit = true;
			FinalDamage *= CriticalDamage;
		}
	}
	UE_LOG(LogTemp,Warning,TEXT(" 최종 데미지 : %f"), FinalDamage);

	// 최종 데미지를 Damage 어트리뷰트에
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UCAP_AttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
}
