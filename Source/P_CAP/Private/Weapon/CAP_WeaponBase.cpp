// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CAP_WeaponBase.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GameFramework/RotatingMovementComponent.h"

ACAP_WeaponBase::ACAP_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComp = CreateDefaultSubobject<USceneComponent>("Root Component");
	SetRootComponent(RootComp);
	
	InteractionSphere = CreateDefaultSubobject<USphereComponent>("Interaction Collision Component");
	InteractionSphere->SetupAttachment(RootComp);

	MeshContainer = CreateDefaultSubobject<USceneComponent>("Mesh Container");
	MeshContainer->SetupAttachment(RootComp);

	WeaponMesh_R = CreateDefaultSubobject<UStaticMeshComponent>("Weapon Mesh R");
	WeaponMesh_R->SetupAttachment(MeshContainer);
	WeaponMesh_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh_R->SetGenerateOverlapEvents(false);

	WeaponMesh_L = CreateDefaultSubobject<UStaticMeshComponent>("Weapon Mesh L");
	WeaponMesh_L->SetupAttachment(MeshContainer);
	WeaponMesh_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh_L->SetGenerateOverlapEvents(false);

	RotatingMovementComp = CreateDefaultSubobject<URotatingMovementComponent>("Rotating Movement Component");
	RotatingMovementComp->RotationRate = FRotator(0.f, 45.f,0.f);
}


void ACAP_WeaponBase::BeginPlay()
{
	Super::BeginPlay();

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACAP_WeaponBase::OnInteractSphereOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACAP_WeaponBase::OnInteractSphereEndOverlap);
}

void ACAP_WeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	WeaponMesh_R->SetStaticMesh(nullptr);
	WeaponMesh_L->SetStaticMesh(nullptr);
	
	if (WeaponDA)
	{
		for (const FWeaponVisualInfo& VisualInfo : WeaponDA->WeaponVisualInfos)
		{
			if (VisualInfo.EquipHand == EEquipHand::Left)
			{
				WeaponMesh_L->SetStaticMesh(VisualInfo.WeaponMesh);
			}
			else
			{
				WeaponMesh_R->SetStaticMesh(VisualInfo.WeaponMesh);
			}
		}
	}
}

void ACAP_WeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	float DeltaZ = FMath::Sin(GetGameTimeSinceCreation() * BobbingSpeed) * BobbingHeight;
	if (MeshContainer)
	{
		MeshContainer->SetRelativeLocation(FVector(0.f,0.f,DeltaZ));
	}
}


void ACAP_WeaponBase::InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter && WeaponDA)
	{
		PlayerCharacter->PickupWeapon(WeaponDA);
		Destroy();
	}
}

void ACAP_WeaponBase::InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter)
{
	Destroy();
}

void ACAP_WeaponBase::OnInteractSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->SetNearbyInteractable(this);
		PlayerCharacter->UpdateInteractUI(true);
	}
}

void ACAP_WeaponBase::OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->SetNearbyInteractable(nullptr);
		PlayerCharacter->UpdateInteractUI(false);
	}
}

