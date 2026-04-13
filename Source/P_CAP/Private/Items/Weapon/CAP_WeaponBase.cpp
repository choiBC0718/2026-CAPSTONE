// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/CAP_WeaponBase.h"

#include "CAP_WeaponInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Data/CAP_WeaponDataAsset.h"

ACAP_WeaponBase::ACAP_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	WeaponMesh_R = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh R");
	WeaponMesh_R->SetupAttachment(MeshContainer);
	WeaponMesh_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh_R->SetRelativeLocation(FVector(95.f, -25.f, -10.f));
	WeaponMesh_R->SetRelativeRotation(FRotator(0.f, -90.f, -90.f));

	WeaponMesh_L = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh L");
	WeaponMesh_L->SetupAttachment(MeshContainer);
	WeaponMesh_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh_L->SetRelativeLocation(FVector(95.f, 25.f, -10.f));
	WeaponMesh_L->SetRelativeRotation(FRotator(0.f, -90.f, -90.f));
}


void ACAP_WeaponBase::BeginPlay()
{
	Super::BeginPlay();

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
}


void ACAP_WeaponBase::InteractEquip(class ACAP_PlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter && WeaponInstance)
	{
		UCAP_WeaponComponent* WeaponComp = PlayerCharacter->GetWeaponComponent();
		if (WeaponComp)
		{
			WeaponComp->PickupWeapon(WeaponInstance);
			Destroy();
		}
	}
}

void ACAP_WeaponBase::InteractDisassemble(class ACAP_PlayerCharacter* PlayerCharacter)
{
	Destroy();
}
