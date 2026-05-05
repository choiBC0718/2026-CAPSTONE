// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Pet/CAP_FlyingPetPawn.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "P_CAP/P_CAP.h"

// Sets default values
ACAP_FlyingPetPawn::ACAP_FlyingPetPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComponent");
	RootComponent = SphereComp;

	SphereComp->SetCollisionObjectType(ECC_Pet);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	SphereComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMeshComponent");
	MeshComp->SetupAttachment(SphereComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeRotation(FRotator(0.f,-90.f,0.f));
	MeshComp->SetRelativeScale3D(FVector(0.4f,0.4f,0.4f));

	MovementComp = CreateDefaultSubobject<UFloatingPawnMovement>("MovementComponent");

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	BaseDamageTag = UCAP_AbilitySystemStatics::GetDataDamageBaseTag();
	DamageMultiplierTag = UCAP_AbilitySystemStatics::GetDataDamageMultiplierTag();
}

// Called when the game starts or when spawned
void ACAP_FlyingPetPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ACAP_FlyingPetPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!Player)
		return;
	FVector TargetLoc = Player->GetActorLocation() + Player->GetActorRotation().RotateVector(FollowOffset);

	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	if (Distance > TeleportDistance)
	{
		SetActorLocation(TargetLoc, false);
		SetActorRotation(Player->GetActorRotation());
		return;
	}

	FVector NewLoc = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, FollowSpeed);
	FRotator NewRot = FMath::RInterpTo(GetActorRotation(), Player->GetActorRotation(), DeltaTime, FollowSpeed);

	SetActorLocationAndRotation(NewLoc, NewRot, true);
}

void ACAP_FlyingPetPawn::InitializePet(AActor* InPlayer)
{
	Player = InPlayer;
}

void ACAP_FlyingPetPawn::ExecuteAttack(AActor* TargetActor)
{
	if (!Player || !TargetActor)
		return;

	UCAP_AbilitySystemComponent* OwnerASC = Cast<UCAP_AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player));
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!OwnerASC || !TargetASC)
		return;

	TSubclassOf<UGameplayEffect> DamageGE = OwnerASC->GetGenerics()->GetInstantDamageGE(PetDamageType);
	if (!DamageGE)
		return;

	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddInstigator(Player,this);

	FGameplayEffectSpecHandle SpecHandle = OwnerASC->MakeOutgoingSpec(DamageGE, 1.f, Context);
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(BaseDamageTag, PetBaseDamage);
		SpecHandle.Data->SetSetByCallerMagnitude(DamageMultiplierTag, PetDamageMultiplier);

		OwnerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void ACAP_FlyingPetPawn::SetVisualAndOffset(class USkeletalMesh* InMesh, FVector InOffset)
{
	if (MeshComp && InMesh)
	{
		MeshComp->SetSkeletalMesh(InMesh);
	}
	FollowOffset = InOffset;
}

void ACAP_FlyingPetPawn::SetStats(ESkillDamageType InDamageType, float InBaseDamage, float InDamageMultiplier)
{
	PetDamageMultiplier = InDamageMultiplier;
	PetDamageType = InDamageType;
	PetBaseDamage = InBaseDamage;
}
