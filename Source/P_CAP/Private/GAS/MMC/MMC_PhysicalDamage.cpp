// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC/MMC_PhysicalDamage.h"

#include "GAS/Setting/CAP_AttributeSet.h"

UMMC_PhysicalDamage::UMMC_PhysicalDamage()
{
	PhysicalDamageCapture.AttributeToCapture = UCAP_AttributeSet::GetPhysicalDamageAttribute();
	PhysicalDamageCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	PhysicalPenetrationCapture.AttributeToCapture = UCAP_AttributeSet::GetPhysicalPenetrationAttribute();
	PhysicalPenetrationCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	PhysicalArmorCapture.AttributeToCapture = UCAP_AttributeSet::GetPhysicalArmorAttribute();
	PhysicalArmorCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	RelevantAttributesToCapture.Add(PhysicalDamageCapture);
	RelevantAttributesToCapture.Add(PhysicalPenetrationCapture);
	RelevantAttributesToCapture.Add(PhysicalArmorCapture);
}

float UMMC_PhysicalDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedSourceTags.GetAggregatedTags();

	float PhysicalDamage = 0.f;
	float PhysicalPen = 0.f;
	float PhysicalArmor = 0.f;
	GetCapturedAttributeMagnitude(PhysicalDamageCapture, Spec,EvalParams,PhysicalDamage);
	GetCapturedAttributeMagnitude(PhysicalPenetrationCapture, Spec,EvalParams,PhysicalPen);
	GetCapturedAttributeMagnitude(PhysicalArmorCapture, Spec,EvalParams,PhysicalArmor);

	PhysicalDamage = FMath::Max<float>(0.f, PhysicalDamage);
	float Diff = PhysicalPen - PhysicalArmor;
	
	// 이 숫자가 100.0f이면, 차이가 100 날 때 데미지가 2배(또는 0.5배)가 됩니다.
	// 숫자를 50.0f로 낮추면 50만 차이나도 2배가 되니 게임이 훨씬 치명적으로 변합니다.
	const float ScaleFactor = 100.0f; 
	float DamageMultiplier = 1.0f;

	// 3. 차이에 따른 증폭/감소 배율 계산
	if (Diff >= 0.0f)
	{
		// 관통력이 방어력을 뚫었을 때 -> 데미지 폭발적 증폭 (1.0 이상)
		DamageMultiplier = 1.0f + (Diff / ScaleFactor);
	}
	else
	{
		// 방어력이 관통력보다 높을 때 -> 데미지 감소 (1.0 미만, 마이너스 방지)
		DamageMultiplier = ScaleFactor / (ScaleFactor - Diff);
	}

	// 4. 최종 데미지 산출
	float FinalDamage = PhysicalDamage * DamageMultiplier;
	
	//float SkillMultiplier = Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.Skill.Multiplier"), false, 1.0f);
	//FinalDamage *= SkillMultiplier;

	return FinalDamage;
}
