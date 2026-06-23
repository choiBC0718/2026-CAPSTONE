// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC/ExecCalc_MagicalDamage.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "GAS/Setting/CAP_AttributeSet.h"
#include "GAS/Setting/CAP_GameplayAbilityTypes.h"

UExecCalc_MagicalDamage::UExecCalc_MagicalDamage()
{
	MagicalDamageCapture.AttributeToCapture = UCAP_AttributeSet::GetMagicalDamageAttribute();
	MagicalDamageCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	MagicalPenetrationCapture.AttributeToCapture = UCAP_AttributeSet::GetMagicalPenetrationAttribute();
	MagicalPenetrationCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	MagicalArmorCapture.AttributeToCapture = UCAP_AttributeSet::GetMagicalArmorAttribute();
	MagicalArmorCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;

	CriticalChanceCapture.AttributeToCapture = UCAP_AttributeSet::GetCriticalChanceAttribute();
	CriticalChanceCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	CriticalDamageCapture.AttributeToCapture = UCAP_AttributeSet::GetCriticalDamageAttribute();
	CriticalDamageCapture.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;

	RelevantAttributesToCapture.Add(MagicalDamageCapture);
	RelevantAttributesToCapture.Add(MagicalPenetrationCapture);
	RelevantAttributesToCapture.Add(MagicalArmorCapture);
	RelevantAttributesToCapture.Add(CriticalChanceCapture);
	RelevantAttributesToCapture.Add(CriticalDamageCapture);

	DamageMultiplierDataTag = UCAP_AbilitySystemStatics::GetDataDamageMultiplierTag();
	DamageBaseDataTag = UCAP_AbilitySystemStatics::GetDataDamageBaseTag();
	ChargeMultiplierDataTag = UCAP_AbilitySystemStatics::GetAbilityChargeTimeTag();

	CriticalTriggerTag = UCAP_AbilitySystemStatics::GetItemTriggerHitCritical();
}

void UExecCalc_MagicalDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	float MagicalDamage = 0.f;
	float MagicalPen = 0.f;
	float MagicalArmor = 0.f;
	float CriticalChance = 0.f;
	float CriticalDamage = 0.f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicalDamageCapture, EvalParams, MagicalDamage);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicalPenetrationCapture, EvalParams, MagicalPen);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(MagicalArmorCapture, EvalParams, MagicalArmor);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalChanceCapture, EvalParams, CriticalChance);
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalDamageCapture, EvalParams, CriticalDamage);
	
	MagicalDamage = FMath::Max<float>(0.f, MagicalDamage);
	CriticalDamage = FMath::Max<float>(1.0f, CriticalDamage);

	// 설정한 데미지 기본 값 가져오기 (덧셈 연산)
	float BaseDamage = Spec.GetSetByCallerMagnitude(DamageBaseDataTag, false, 0.f);
	// 설정한 데미지 배율 값 가져오기 (곱 연산)
	float DamageMultiplier = Spec.GetSetByCallerMagnitude(DamageMultiplierDataTag, false, 1.0f);
	// 차징 로직에서 보낸 차징 시간 값 가져오기 (곱 연산)
	float ChargeMultiplier = Spec.GetSetByCallerMagnitude(ChargeMultiplierDataTag, false, 1.0f);

	// 기초 공격력 계산
	float TotalAttackPow = (MagicalDamage * DamageMultiplier * ChargeMultiplier) + BaseDamage;
	
	// 방어력 경감 계산 
	float Diff = MagicalPen - MagicalArmor;
	float DiffAmp = (Diff >= 0.0f) ? (1.0f + (Diff / 100.0f)) : (100.0f / (100.0f - Diff));
	float FinalDamage = TotalAttackPow * DiffAmp;
	
	// 크리티컬 계산
	bool bCriticalHit = false;
	if (CriticalChance > 0.0f)
	{
		if (FMath::RandRange(0.0f, 100.0f) <= CriticalChance)
		{
			FinalDamage *= CriticalDamage;
			bCriticalHit = true;
			
			AActor* SourceActor = ExecutionParams.GetSourceAbilitySystemComponent()->GetAvatarActor();
			AActor* TargetActor = ExecutionParams.GetTargetAbilitySystemComponent()->GetAvatarActor();
			
			if (SourceActor && TargetActor)
			{
				FGameplayEventData CritPayload;
				CritPayload.EventTag = CriticalTriggerTag;
				CritPayload.Instigator = SourceActor;
				CritPayload.Target = TargetActor;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(SourceActor, CritPayload.EventTag, CritPayload);
			}
		}
	}
	
	if (FCAP_GameplayEffectContext* CContext = static_cast<FCAP_GameplayEffectContext*>(Spec.GetContext().Get()))
		CContext->bIsCritical = bCriticalHit;
	
	FinalDamage = FMath::RoundToInt(FinalDamage);
	//UE_LOG(LogTemp,Warning,TEXT(" 최종 데미지 : %f"), FinalDamage);

	// 최종 데미지를 Damage 어트리뷰트에
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UCAP_AttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, FinalDamage));
}
