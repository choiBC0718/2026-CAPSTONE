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

	USceneComponent* TargetMesh = nullptr;
	if (bUseCharacterBones)
		TargetMesh = MeshComp;
	else
		TargetMesh = GetWeaponMesh(MeshComp);
	
	if (TargetMesh)
	{
		for (const FName& SocketName : TargetSocketNames)
		{
			// 메쉬의 소켓의 위치를 PrevSocketLocationMap의 Value에 추가
			PrevLocs.Add(TargetMesh->GetSocketLocation(SocketName));
		}
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

	USceneComponent* TargetMesh = nullptr;
	if (bUseCharacterBones)
		TargetMesh = MeshComp;
	else
		TargetMesh = GetWeaponMesh(MeshComp);
	
	if (!TargetMesh)
		return;

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

		// 1프레임 동안 맞은 적들을 모아두는 배열
		TArray<FHitResult> CombinedHitResults;

		// 이전 프레임의 칼 끝 ~ 현재 프레임의 칼 끝 사이의 거리
		float MoveDistance = FVector::Distance(PrevEndLoc, CurrentEndLoc);
		
		// 프레임 드랍 시 트레이스 간 벌어지는 공간을 몇번 메울지
		int32 TraceCount = FMath::RoundToInt(MoveDistance / (SweepRadius * 1.5f));
		TraceCount = FMath::Clamp(TraceCount, 1, 10);
		// 프레임 사이에 빈 곳을 채움
		for (int32 Step = 1; Step <= TraceCount; ++Step)
		{
			float Alpha = (float)Step / TraceCount;

			FVector LerpStart = FMath::Lerp(PrevStartLoc, CurrentStartLoc, Alpha);
			FVector LerpEnd = FMath::Lerp(PrevEndLoc, CurrentEndLoc, Alpha);
			// 하나의 트레이스에 적중된 대상 임시 저장소
			TArray<FHitResult> TempHits;
			UKismetSystemLibrary::CapsuleTraceMultiForObjects(
				MeshComp, 
				LerpStart, LerpEnd, SweepRadius, SweepHalfHeight,
				ObjectTypes, false, IgnoreActors, DrawDebugType, TempHits, true
			);
			// 임시 저장소의 대상들을 전체 배열에 담아
			CombinedHitResults.Append(TempHits);
		}

		// 전체 적중 대상에 대해 반복
		for (const FHitResult& Hit : CombinedHitResults)
		{
			AActor* HitActor = Hit.GetActor();
			// 적중 대상 배열에 아직 추가를 하지 않았으면 데미지 이벤트 보내
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

		// 현재 위치를 이전 위치로 갱신
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
	}
	if (MeshComp)
	{
		HitActorsMap.Remove(MeshComp);
		PrevSocketLocationMap.Remove(MeshComp);
		TargetChannelMap.Remove(MeshComp);
	}
}

class USkeletalMeshComponent* UANS_MeleeHitCheck::GetWeaponMesh(USkeletalMeshComponent* CharacterMesh) const
{
	if (!CharacterMesh || !CharacterMesh->GetOwner())
		return nullptr;

	TArray<USkeletalMeshComponent*> WeaponMeshes;

	// ANS호출한 캐릭터에서 특정 컴포넌트 태그 설정되어있는 StaticMeshComponent 가져오기
	CharacterMesh->GetOwner()->GetComponents<USkeletalMeshComponent>(WeaponMeshes);
	// 설정한 변수에 따라 가져오는 메시 컴포넌트 다르게
	FName TagToFind = (WeaponHand == EEquipHand::Right) ? "RightHand" : "LeftHand";
	for (USkeletalMeshComponent* Mesh : WeaponMeshes)
	{
		if (Mesh->ComponentHasTag(TagToFind))
		{
			return Mesh;
		}
	}
	return nullptr;
}
