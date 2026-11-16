// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_MeleeHitCheck.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "P_CAP/P_CAP.h"

UANS_MeleeHitCheck::UANS_MeleeHitCheck()
{
	TargetSocketNames.Add("FX_WeaponBase_R");
	TargetSocketNames.Add("FX_WeaponTip_R");

	EventTag = UCAP_AbilitySystemStatics::GetAnimHitTag();
}

void UANS_MeleeHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp) return;

	// 공격자 캐릭터 Mesh 저장 (적중 대상은 빈칸)
	HitActorsMap.Add(MeshComp, TSet<AActor*>());
	TArray<FVector>& PrevLocs = PrevSocketLocationMap.Add(MeshComp, TArray<FVector>());

	// 파라곤 에셋은 무기가 스켈레톤에 포함되어 있으므로 MeshComp를 바로 타겟으로 사용
	USceneComponent* TargetMesh = MeshComp;
	
	for (const FName& SocketName : TargetSocketNames)
	{
		// 메쉬의 소켓의 위치를 PrevSocketLocationMap의 Value에 추가
		PrevLocs.Add(TargetMesh->GetSocketLocation(SocketName));
	}
	
	AActor* OwnerActor = MeshComp->GetOwner();
	if (OwnerActor)
	{
		uint8 TeamIdVal = 255;
		if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(OwnerActor))
			TeamIdVal = TeamAgent->GetGenericTeamId().GetId();
		
		ECollisionChannel TargetChannel = (TeamIdVal == 0) ? ECC_EnemyHitbox : ECC_PlayerHitbox;
		TargetChannelMap.Add(MeshComp, TargetChannel);
	}
}

void UANS_MeleeHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!MeshComp || TargetSocketNames.Num() < 2)
		return;

	// 메인 메쉬를 그대로 타겟으로 사용
	USceneComponent* TargetMesh = MeshComp;

	TSet<AActor*>* HitActors = HitActorsMap.Find(MeshComp);
	TArray<FVector>* PrevLocs = PrevSocketLocationMap.Find(MeshComp);
	ECollisionChannel* CachedChannel = TargetChannelMap.Find(MeshComp);

	if (!HitActors || !CachedChannel || !PrevLocs || PrevLocs->Num() != TargetSocketNames.Num())
		return;

	AActor* OwnerActor = MeshComp->GetOwner();
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(OwnerActor);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(*CachedChannel));

	EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	for (int32 i = 1; i < TargetSocketNames.Num(); ++i)
	{
		// 현재 프레임의 시작-끝 위치
		FVector CurrentStartLoc = TargetMesh->GetSocketLocation(TargetSocketNames[i - 1]);
		FVector CurrentEndLoc = TargetMesh->GetSocketLocation(TargetSocketNames[i]);
		//이전 프레임의 시작-끝 위치
		FVector PrevStartLoc = (*PrevLocs)[i - 1];
		FVector PrevEndLoc = (*PrevLocs)[i];

		TArray<FHitResult> CombinedHitResults;
		float MoveDistance = FVector::Distance(PrevEndLoc, CurrentEndLoc);
		
		int32 TraceCount = FMath::RoundToInt(MoveDistance / (SweepRadius * 1.5f));
		TraceCount = FMath::Clamp(TraceCount, 1, 10);
		
		for (int32 Step = 1; Step <= TraceCount; ++Step)
		{
			float Alpha = (float)Step / TraceCount;

			FVector LerpStart = FMath::Lerp(PrevStartLoc, CurrentStartLoc, Alpha);
			FVector LerpEnd = FMath::Lerp(PrevEndLoc, CurrentEndLoc, Alpha);
			
			TArray<FHitResult> TempHits;
			UKismetSystemLibrary::CapsuleTraceMultiForObjects(
				MeshComp, 
				LerpStart, LerpEnd, SweepRadius, SweepHalfHeight,
				ObjectTypes, false, IgnoreActors, DrawDebugType, TempHits, true
			);
			CombinedHitResults.Append(TempHits);
		}

		for (const FHitResult& Hit : CombinedHitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !HitActors->Contains(HitActor))
			{
				HitActors->Add(HitActor); 

				FGameplayEventData Payload;
				Payload.Instigator = OwnerActor;
				Payload.Target = HitActor;

				FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(Hit);
				Payload.TargetData.Add(TargetData);

				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OwnerActor, EventTag, Payload);
			}
		}

		(*PrevLocs)[i - 1] = CurrentStartLoc;
		if (i == TargetSocketNames.Num() - 1)
		{
			(*PrevLocs)[i] = CurrentEndLoc;
		}
	}
}

void UANS_MeleeHitCheck::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp->GetWorld()->IsPreviewWorld())
		return;
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (MeshComp)
	{
		HitActorsMap.Remove(MeshComp);
		PrevSocketLocationMap.Remove(MeshComp);
		TargetChannelMap.Remove(MeshComp);
	}
}