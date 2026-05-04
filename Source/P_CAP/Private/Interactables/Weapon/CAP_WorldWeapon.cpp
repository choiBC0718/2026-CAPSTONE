// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Weapon/CAP_WorldWeapon.h"

#include "CAP_WeaponInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_WeaponComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GameFramework/RotatingMovementComponent.h"

ACAP_WorldWeapon::ACAP_WorldWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshContainer=CreateDefaultSubobject<USceneComponent>(TEXT("MeshContainer"));
	MeshContainer->SetupAttachment(GetRootComponent());
	
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

	RotatingMovement=CreateDefaultSubobject<URotatingMovementComponent>("RotatingMovement");
	RotatingMovement->SetUpdatedComponent(MeshContainer);
	RotatingMovement->RotationRate=FRotator(0.f,45.f,0.f);
}


void ACAP_WorldWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 데이터 에셋(설계도)를 가지고 Instance(같은 무기더라도 다른 데이터를 가진) 생성
	if (!WeaponInstance && WeaponDA)
	{
		WeaponInstance = NewObject<UCAP_WeaponInstance>(this);
		WeaponInstance->InitializeWeapon(WeaponDA);
		WeaponInstance->LoadWeaponAssets(FStreamableDelegate::CreateLambda([](){}));
	}
}

void ACAP_WorldWeapon::OnConstruction(const FTransform& Transform)
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
				WeaponMesh_L->SetSkeletalMesh(VisualInfo.WeaponMesh.LoadSynchronous());
			else
				WeaponMesh_R->SetSkeletalMesh(VisualInfo.WeaponMesh.LoadSynchronous());
		}
	}
}

void ACAP_WorldWeapon::Interact(class AActor* InsActor, EInteractAction ActionType)
{
	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor))
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			WeaponComp->PickupWeapon(WeaponInstance);
			Destroy();
		}
	}
}

FInteractionPayload ACAP_WorldWeapon::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	Payload.DetailData = WeaponInstance;
	Payload.ActionData.ShortActionText = TEXT("줍기");
	Payload.ActionData.LongActionText = TEXT("파괴하기");
	Payload.ActionData.bShowCurrency = true;
	Payload.ActionData.ActionCurrencyType = ECurrencyType::WeaponMaterial;
	Payload.ActionData.CurrencyAmount = 5; 
	return Payload;
}