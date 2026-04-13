// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/CAP_DropItemBase.h"

#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

ACAP_DropItemBase::ACAP_DropItemBase()
{
	PrimaryActorTick.bCanEverTick = true;

	RootCollision= CreateDefaultSubobject<USphereComponent>("Root Collision Comp");
	SetRootComponent(RootCollision);
	RootCollision->SetSphereRadius(20.f);
	RootCollision->SetSimulatePhysics(true);
	RootCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RootCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	RootCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);

	RootCollision->BodyInstance.bLockXRotation = true;
	RootCollision->BodyInstance.bLockYRotation = true;
	RootCollision->BodyInstance.bLockZRotation = true;

	InteractionSphere = CreateDefaultSubobject<USphereComponent>("Interaction Sphere");
	InteractionSphere->SetupAttachment(RootCollision);
	InteractionSphere->SetSphereRadius(80.f);
	InteractionSphere->SetRelativeLocation(FVector(0.f,0.f,100.f));

	MeshContainer = CreateDefaultSubobject<USceneComponent>("MeshContainer");
	MeshContainer->SetupAttachment(InteractionSphere);

	RotatingMovementComp = CreateDefaultSubobject<URotatingMovementComponent>("Rotating Movement Comp");
	RotatingMovementComp->RotationRate = FRotator(0.f, 45.f, 0.f);
	RotatingMovementComp->SetUpdatedComponent(MeshContainer);
}


void ACAP_DropItemBase::BeginPlay()
{
	Super::BeginPlay();

	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACAP_DropItemBase::OnInteractSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACAP_DropItemBase::OnInteractSphereEndOverlap);
	
	RootCollision->OnComponentHit.AddDynamic(this, &ACAP_DropItemBase::OnDropCollisionHit);
}

void ACAP_DropItemBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (MeshContainer)
	{
		float DeltaZ = FMath::Sin(GetGameTimeSinceCreation() * BobbingSpeed) * BobbingHeight;
		MeshContainer->SetRelativeLocation(FVector(0.f,0.f,DeltaZ));
	}
}

void ACAP_DropItemBase::InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter)
{
	// 자식 클래스에서 override
}

void ACAP_DropItemBase::InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter)
{
	// 자식 클래스에서 override
}

void ACAP_DropItemBase::DropItem()
{
	if (RootCollision)
	{
		FVector DropImpulse = FVector(0.f,0.f, 600.f);
		RootCollision->AddImpulse(DropImpulse, NAME_None, true);
	}
}

void ACAP_DropItemBase::OnInteractSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetInventoryComponent()->SetNearbyInteractable(this);
		PlayerCharacter->UpdateInteractUI(true);
	}
}

void ACAP_DropItemBase::OnInteractSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ACAP_PlayerCharacter* PlayerCharacter = Cast<ACAP_PlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		PlayerCharacter->GetInventoryComponent()->SetNearbyInteractable(nullptr);
		PlayerCharacter->UpdateInteractUI(false);
	}
}

void ACAP_DropItemBase::OnDropCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	RootCollision->SetSimulatePhysics(false);
}
