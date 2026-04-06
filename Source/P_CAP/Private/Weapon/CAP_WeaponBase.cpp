// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CAP_WeaponBase.h"

#include "CAP_WeaponInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GameFramework/RotatingMovementComponent.h"

ACAP_WeaponBase::ACAP_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	DropCollision = CreateDefaultSubobject<UBoxComponent>("Drop Collision");
	SetRootComponent(DropCollision);
	DropCollision->SetBoxExtent(FVector(100.f,100.f,10.f));
	DropCollision->SetSimulatePhysics(true);
	// 바닥(WorldStatic)과는 충돌해서 튕기게 하고, 캐릭터(Pawn)는 뚫고 지나가게 설정
	DropCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	DropCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	DropCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	DropCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	
	InteractionSphere = CreateDefaultSubobject<USphereComponent>("Interaction Collision Component");
	InteractionSphere->SetupAttachment(DropCollision);
	InteractionSphere->SetSphereRadius(75.f);
	InteractionSphere->SetRelativeLocation(FVector(0.f,0.f,100.f));

	MeshContainer = CreateDefaultSubobject<USceneComponent>("Mesh Container");
	MeshContainer->SetupAttachment(InteractionSphere);

	WeaponMesh_R = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh R");
	WeaponMesh_R->SetupAttachment(MeshContainer);
	WeaponMesh_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh_R->SetGenerateOverlapEvents(false);
	WeaponMesh_R->SetRelativeLocation(FVector(95.f, -25.f, -10.f));
	WeaponMesh_R->SetRelativeRotation(FRotator(0.f, -90.f, -90.f));

	WeaponMesh_L = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh L");
	WeaponMesh_L->SetupAttachment(MeshContainer);
	WeaponMesh_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh_L->SetGenerateOverlapEvents(false);
	WeaponMesh_L->SetRelativeLocation(FVector(95.f, 25.f, -10.f));
	WeaponMesh_L->SetRelativeRotation(FRotator(0.f, -90.f, -90.f));

	RotatingMovementComp = CreateDefaultSubobject<URotatingMovementComponent>("Rotating Movement Component");
	RotatingMovementComp->RotationRate = FRotator(0.f, 45.f,0.f);
	RotatingMovementComp->SetUpdatedComponent(MeshContainer);
}


void ACAP_WeaponBase::BeginPlay()
{
	Super::BeginPlay();
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACAP_WeaponBase::OnInteractSphereOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &ACAP_WeaponBase::OnInteractSphereEndOverlap);

	// 데이터 에셋(설계도)를 가지고 Instance(같은 무기더라도 다른 데이터를 가진) 생성
	if (!WeaponInstance && WeaponDA)
	{
		WeaponInstance = NewObject<UCAP_WeaponInstance>(this);
		WeaponInstance->InitializeWeapon(WeaponDA);
	}
}

void ACAP_WeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!WeaponMesh_L || !WeaponMesh_R)	return;

	WeaponMesh_R->SetSkeletalMesh(nullptr);
	WeaponMesh_L->SetSkeletalMesh(nullptr);

	UCAP_WeaponDataAsset* DAToUse = nullptr;
	// 생성된 Instance가 있다면 그 Instance를 만들 때 사용한 DA 이용
	if (WeaponInstance && WeaponInstance->GetWeaponDA())
	{
		DAToUse = WeaponInstance->GetWeaponDA();
	}// Instance없다면 그냥 설정한 DA 이용
	else if (WeaponDA){
		DAToUse = WeaponDA;
	}

	if (DAToUse)
	{
		for (const FWeaponVisualInfo& VisualInfo : DAToUse->WeaponVisualInfos)
		{
			if (!VisualInfo.WeaponMesh)
				continue;
			
			if (VisualInfo.EquipHand == EEquipHand::Left)
			{
				WeaponMesh_L->SetSkeletalMesh(VisualInfo.WeaponMesh.LoadSynchronous());
			}
			else
			{
				WeaponMesh_R->SetSkeletalMesh(VisualInfo.WeaponMesh.LoadSynchronous());
			}
		}
	}
}

void ACAP_WeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (DropCollision && DropCollision->IsSimulatingPhysics())
	{
		// bAccelChange를 true로 하면 무기의 질량(Mass)을 무시하고 가속도로 직접 적용됩니다.
		// 언리얼 기본 중력(-980)에 추가로 -2000을 더해 약 3배 무겁고 빠르게 툭 떨어집니다!
		DropCollision->AddForce(FVector(0.f, 0.f, -2000.f), NAME_None, true);
	}
	float DeltaZ = FMath::Sin(GetGameTimeSinceCreation() * BobbingSpeed) * BobbingHeight;
	if (MeshContainer)
	{
		MeshContainer->SetRelativeLocation(FVector(0.f,0.f,DeltaZ));
	}
}


void ACAP_WeaponBase::InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter && WeaponInstance)
	{
		PlayerCharacter->PickupWeapon(WeaponInstance);
		Destroy();
	}
}

void ACAP_WeaponBase::InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter)
{
	Destroy();
}

void ACAP_WeaponBase::DropWeapon()
{
	if (DropCollision)
	{
		DropCollision->SetPhysicsLinearVelocity(FVector(0.f, 0.f, 1200.f));
	}
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

