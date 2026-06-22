// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_SpawnProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Engine/OverlapResult.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "P_CAP/P_CAP.h"

void UItemBehavior_SpawnProjectile::OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	BindGameplayEvent(StateProvider,ASC,TriggerEventTag);
}

void UItemBehavior_SpawnProjectile::OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(StateProvider, ASC);
}

void UItemBehavior_SpawnProjectile::OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const
{
	if (!Payload || !ProjectileClass || !ASC || !ASC->GetGenerics()) return;
	if (!CheckTriggerCondition(StateProvider, ASC)) return;

	AActor* Instigator = ASC->GetAvatarActor();
	if (!Instigator) return;
	UWorld* World = Instigator->GetWorld();

	// 1. 시전자 기준 오프셋 및 방향
	FVector BaseLoc = Instigator->GetActorLocation() + Instigator->GetActorRotation().RotateVector(SpawnOffset);
	FVector BaseDir = Instigator->GetActorForwardVector();

	// 2. 데미지 사전 연산 (Pre-calculation)
	float AttributeValue = 0.f;
	if (DamageCalculation.ReferenceAttribute.IsValid())
	{
		AttributeValue = ASC->GetNumericAttribute(DamageCalculation.ReferenceAttribute);
	}
	float FinalPreCalculatedDamage = (AttributeValue * DamageCalculation.Multiplier) + DamageCalculation.BaseValue;

	// 3. GameplayEffect 생성 및 데미지/타입 주입
	FGameplayEffectSpecHandle DamageHandle;
	if (TSubclassOf<UGameplayEffect> DamageGE = ASC->GetGenerics()->GetItemDamageGE())
	{
		DamageHandle = ASC->MakeOutgoingSpec(DamageGE, 1.f, ASC->MakeEffectContext());
		
		// 확정된 기초 데미지 주입
		DamageHandle.Data->SetSetByCallerMagnitude(UCAP_AbilitySystemStatics::GetDataDamageBaseTag(), FinalPreCalculatedDamage);

		// 데미지 타입에 따른 태그 부여 (ExecCalc_ItemDamage 분기용)
		if (DamageCalculation.DamageType == ESkillDamageType::Physical)
			DamageHandle.Data->AddDynamicAssetTag(FGameplayTag::RequestGameplayTag("Damage.Type.Physical"));
		else if (DamageCalculation.DamageType == ESkillDamageType::Magical)
			DamageHandle.Data->AddDynamicAssetTag(FGameplayTag::RequestGameplayTag("Damage.Type.Magical"));
	}

	// 4. 추적 타겟 지정 (호밍 또는 확정 폴링용)
	TArray<USceneComponent*> TrackedTarget;
	if (ProjectileType == EProjectileType::Homing || (ProjectileType == EProjectileType::Falling && !bIsRandomFalling))
	{
		if (Payload->TargetData.Num() > 0 && Payload->TargetData.Data[0]->GetHitResult())
		{
			if (AActor* HitActor = Payload->TargetData.Data[0]->GetHitResult()->GetActor())
				TrackedTarget.Add(HitActor->GetRootComponent());
		}
		
		if (TrackedTarget.Num()==0)
		{
			TrackedTarget = FindNearestTarget(Instigator->GetActorLocation(), World, Instigator);
		}
	}

	// 5. 투사체 스폰 루트 (NumProjectiles 만큼 반복)
	int32 SpawnNums = FMath::Max(1, NumProjectiles);
	for (int32 i = 0; i < SpawnNums; i++)
	{
		FVector SpawnLoc = BaseLoc;
		FVector LaunchDir = BaseDir;
		USceneComponent* CurrentTarget = nullptr;

		if (TrackedTarget.Num() > 0)
			CurrentTarget = TrackedTarget[i%TrackedTarget.Num()];

		// 타입별 위치 및 방향 계산
		if (ProjectileType == EProjectileType::Falling)
		{
			LaunchDir = FVector::DownVector;

			if (bIsRandomFalling)
			{
				// 무작위 메테오: 중심점(BaseLoc) 주변 랜덤
				FVector2D RandPoint = FMath::RandPointInCircle(RandomFallingRadius);
				SpawnLoc = BaseLoc + FVector(RandPoint.X, RandPoint.Y, FallingSpawnHeight);
			}
			else
			{
				// 머리 위 타격: 타겟이 있으면 타겟 위, 없으면 내 앞의 위
				FVector TargetLoc = CurrentTarget ? CurrentTarget->GetComponentLocation() : BaseLoc;
				int32 DropOrder = TrackedTarget.Num() > 0 ? (i / TrackedTarget.Num()) : i;
				
				float HeightStagger = ProjectileSpeed * 0.15f; 
				SpawnLoc = TargetLoc + FVector(0.f, 0.f, FallingSpawnHeight + (DropOrder * HeightStagger));
			}
		}
		else // Straight or Homing
		{
			if (SpawnNums > 1 && SpreadAngle > 0.f)
			{
				float HalfAngle = SpreadAngle / 2.f;
				float Step = SpawnNums > 1 ? SpreadAngle / (SpawnNums - 1) : 0.f;
				float CurrentAngle = -HalfAngle + (i * Step);
				LaunchDir = BaseDir.RotateAngleAxis(CurrentAngle, FVector::UpVector);
			}
		}

		FTransform SpawnTransform(LaunchDir.Rotation(), SpawnLoc);
		ACAP_ProjectileBase* Projectile = World->SpawnActorDeferred<ACAP_ProjectileBase>(
			ProjectileClass, SpawnTransform, Instigator, Cast<APawn>(Instigator), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		if (Projectile)
		{
			FProjectileInitData InitData;
			InitData.ProjectileType = ProjectileType;
			InitData.LaunchDir = LaunchDir;
			InitData.ProjectileSpeed = ProjectileSpeed;
			InitData.MaxDistance = MaxDistance;
			InitData.ExplosionRadius = ExplosionRadius;
			InitData.MaxHitCount = MaxHitCount;
			InitData.HomingTarget = CurrentTarget;
			InitData.DamageSpecHandle = DamageHandle;
			InitData.CueTag = HitCueTag;
			InitData.HitTriggerTag = FGameplayTag::RequestGameplayTag("Item.Trigger.Hit.Item");

			Projectile->InitProjectile(InitData);
			Projectile->FinishSpawning(SpawnTransform);
		}
	}
	// 아이템 캐스팅 이벤트 캐스트
	FGameplayEventData CastPayload;
	CastPayload.Instigator = Instigator;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Instigator, FGameplayTag::RequestGameplayTag("Item.Trigger.Cast.Item"), CastPayload);
}

bool UItemBehavior_SpawnProjectile::CheckTriggerCondition(ICAP_BehaviorStateProvider* StateProvider,UCAP_AbilitySystemComponent* ASC) const
{
	if (IsOnCooldown(StateProvider, ASC))
		return false;
	if (FMath::RandRange(0.f, 100.f) > TriggerChance)
		return false;

	StateProvider->AddBehaviorCount(this,1);
	int32 CurrentCount = StateProvider->GetBehaviorCount(this);

	bool bWillFire = (CurrentCount >= RequiredTriggerCount);
	if (!bWillFire)
	{
		if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
		{
			if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
				InvComp->OnItemEffectTriggered.Broadcast(StateProvider->GetProviderObject(),BehaviorTag,0.f,0.f,CurrentCount);
		}
		return false;
	}

	ConsumeCooldown(StateProvider, ASC);
	StateProvider->ResetBehaviorCount(this);
	
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			InvComp->OnItemEffectTriggered.Broadcast(StateProvider->GetProviderObject(), BehaviorTag, Cooldown, 0.f, 0);
	}
	return true;
}

TArray<USceneComponent*> UItemBehavior_SpawnProjectile::FindNearestTarget(const FVector& Origin, class UWorld* World, AActor* IgnoredActor) const
{
	TArray<USceneComponent*> ResultTargets;
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjQueryParams;
	
	uint8 TeamIdVal = 255;
	if (IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(IgnoredActor))
		TeamIdVal = TeamAgent->GetGenericTeamId().GetId();
	
	ECollisionChannel TargetChannel = (TeamIdVal == 0) ? ECC_EnemyHitbox : ECC_PlayerHitbox;
	ObjQueryParams.AddObjectTypesToQuery(TargetChannel);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(SearchRadius);

	if (World->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, ObjQueryParams, SphereShape))
	{
		struct FTargetDist { USceneComponent* Comp; float DistSq; };
		TArray<FTargetDist> TargetDists;

		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* Enemy = Overlap.GetActor();
			if (Enemy && Enemy != IgnoredActor)
			{
				float DistSq = FVector::DistSquared(Origin, Enemy->GetActorLocation());
				TargetDists.Add({Enemy->GetRootComponent(), DistSq});
			}
		}

		TargetDists.Sort([](const FTargetDist& A, const FTargetDist& B) {
			return A.DistSq < B.DistSq;
		});

		for (const FTargetDist& TD : TargetDists)
		{
			ResultTargets.Add(TD.Comp);
		}
	}
	return ResultTargets;
}
