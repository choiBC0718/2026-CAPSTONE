// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ANS_MeleeHitCheck.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Kismet/KismetSystemLibrary.h"

UANS_MeleeHitCheck::UANS_MeleeHitCheck()
{
	TargetSocketNames.Add("Hit_Start");
	TargetSocketNames.Add("Hit_End");
}

void UANS_MeleeHitCheck::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp) return;
	
	HitActorsMap.Add(MeshComp, TSet<AActor*>());
	TArray<FVector>& PrevLocs = PrevSocketLocationMap.Add(MeshComp, TArray<FVector>());

	UStaticMeshComponent* WeaponMesh = GetWeaponMesh(MeshComp);
	if (WeaponMesh)
	{
		for (const FName& SocketName : TargetSocketNames)
		{
			PrevLocs.Add(WeaponMesh->GetSocketLocation(SocketName));
		}
	}
}

void UANS_MeleeHitCheck::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	
	if (!MeshComp || TargetSocketNames.Num() < 2)
		return;

	UStaticMeshComponent* WeaponMesh = GetWeaponMesh(MeshComp);
	if (!WeaponMesh)
		return;

	TSet<AActor*>* HitActors = HitActorsMap.Find(MeshComp);
	TArray<FVector>* PrevLocs = PrevSocketLocationMap.Find(MeshComp);

	if (!HitActors || !PrevLocs || PrevLocs->Num() != TargetSocketNames.Num())
		return;

	AActor* OwnerActor = MeshComp->GetOwner();
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(OwnerActor);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	EDrawDebugTrace::Type DrawDebugType = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	for (int32 i = 1; i < TargetSocketNames.Num(); ++i)
	{
		FVector CurrentStartLoc = WeaponMesh->GetSocketLocation(TargetSocketNames[i - 1]);
		FVector CurrentEndLoc = WeaponMesh->GetSocketLocation(TargetSocketNames[i]);
		FVector PrevStartLoc = (*PrevLocs)[i - 1];
		FVector PrevEndLoc = (*PrevLocs)[i];

		TArray<FHitResult> CombinedHitResults;
		
		float MoveDistance = FVector::Distance(PrevEndLoc, CurrentEndLoc);
		
		int32 TraceCount = FMath::RoundToInt(MoveDistance / (SphereSweepRadius * 1.5f));
		TraceCount = FMath::Clamp(TraceCount, 1, 10);
		
		for (int32 Step = 1; Step <= TraceCount; ++Step)
		{
			float Alpha = (float)Step / TraceCount;

			// 프레임 드랍 시 트레이스 끊기는 문제 방지용 Lerp
			FVector LerpStart = FMath::Lerp(PrevStartLoc, CurrentStartLoc, Alpha);
			FVector LerpEnd = FMath::Lerp(PrevEndLoc, CurrentEndLoc, Alpha);

			TArray<FHitResult> TempHits;
			UKismetSystemLibrary::SphereTraceMultiForObjects(MeshComp, LerpStart, LerpEnd, SphereSweepRadius, ObjectTypes, false, IgnoreActors, DrawDebugType, TempHits, true);
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
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if (MeshComp)
	{
		HitActorsMap.Remove(MeshComp);
		PrevSocketLocationMap.Remove(MeshComp);
	}
}

class UStaticMeshComponent* UANS_MeleeHitCheck::GetWeaponMesh(USkeletalMeshComponent* CharacterMesh) const
{
	if (!CharacterMesh || !CharacterMesh->GetOwner())
		return nullptr;

	TArray<UStaticMeshComponent*> WeaponMeshes;

	// ANS호출한 캐릭터에서 특정 컴포넌트 태그 설정되어있는 StaticMeshComponent 가져오기
	CharacterMesh->GetOwner()->GetComponents<UStaticMeshComponent>(WeaponMeshes);
	// 설정한 변수에 따라 가져오는 메시 컴포넌트 다르게
	FName TagToFind = (WeaponHand == EEquipHand::Right) ? "RightHand" : "LeftHand";
	for (UStaticMeshComponent* Mesh : WeaponMeshes)
	{
		if (Mesh->ComponentHasTag(TagToFind))
		{
			return Mesh;
		}
	}
	return nullptr;
}
