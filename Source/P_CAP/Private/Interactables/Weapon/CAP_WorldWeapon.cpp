// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Weapon/CAP_WorldWeapon.h"

#include "CAP_WeaponInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_WeaponComponent.h"
#include "Components/SphereComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "Framework/CAP_RewardSettings.h"
#include "GameFramework/RotatingMovementComponent.h"

ACAP_WorldWeapon::ACAP_WorldWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	RootCollision = CreateDefaultSubobject<USphereComponent>("RootCollision");
	SetRootComponent(RootCollision);
	RootCollision->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	RootCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootCollision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	RootCollision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	RootCollision->SetSimulatePhysics(true);

	RootScene->SetupAttachment(RootCollision);
	
	MeshContainer=CreateDefaultSubobject<USceneComponent>("MeshContainer");
	MeshContainer->SetupAttachment(InteractionSphere);
	
	WeaponMesh_R = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh R");
	WeaponMesh_R->SetupAttachment(MeshContainer);
	WeaponMesh_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh_L = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh L");
	WeaponMesh_L->SetupAttachment(MeshContainer);
	WeaponMesh_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
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
	RootCollision->OnComponentHit.AddDynamic(this, &ACAP_WorldWeapon::OnRootCollisionHit);
	
	if (WeaponInstance)
	{
		const UCAP_RewardSettings* RewardSetting = GetDefault<UCAP_RewardSettings>();
		if (RewardSetting->DisassembleRewardDT.IsNull())
			return;

		if (UDataTable* LoadedDT = RewardSetting->DisassembleRewardDT.LoadSynchronous())
		{
			FName Grade = RewardSetting->GetRowNameFromGrade(WeaponInstance->GetCurrentGrade());
			if (const FDisassembleRewardRow* RewardRow = LoadedDT->FindRow<FDisassembleRewardRow>(Grade,""))
			{
				CachedBaseRewardAmount = RewardRow->WeaponRewardAmount;
				CachedRewardCurrencyType = RewardRow->WeaponCurrencyType;
			}
		}
	}
}

void ACAP_WorldWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!WeaponMesh_L || !WeaponMesh_R)	return;
/*
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
	}*/

	if (WeaponDA && !WeaponDA->WeaponVisualInfos.IsEmpty())
	{
		for (const FWeaponVisualInfo& VisualInfo : WeaponDA->WeaponVisualInfos)
		{
			if (VisualInfo.EquipHand == EEquipHand::Left)
			{
				WeaponMesh_L->SetSkeletalMesh(VisualInfo.WeaponMesh.LoadSynchronous());
				WeaponMesh_L->SetRelativeTransform(VisualInfo.ConstructionOffset);
			}
			else
			{
				WeaponMesh_R->SetSkeletalMesh(VisualInfo.WeaponMesh.LoadSynchronous());
				WeaponMesh_R->SetRelativeTransform(VisualInfo.ConstructionOffset);
			}
		}
	}
}

void ACAP_WorldWeapon::Interact(class AActor* InsActor, EInteractAction ActionType)
{
	ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(InsActor);
	if (!Player) return;
	
	if (ActionType == EInteractAction::Tap)
	{
		if (UCAP_WeaponComponent* WeaponComp = Player->GetWeaponComponent())
		{
			WeaponComp->PickupWeapon(WeaponInstance);
			Destroy();
		}
	}
	else
	{
		if (WeaponInstance)
		{
			if (UCAP_CurrencyComponent* CurrencyComp = Player->GetCurrencyComponent())
			{
				EItemGrade TargetGrade = WeaponInstance->GetCurrentGrade();
				CurrencyComp->ProcessDisassembleReward(TargetGrade, ECurrencyType::WeaponMaterial);
			}
		}
		Destroy();
	}
}

FInteractionPayload ACAP_WorldWeapon::GetInteractionPayload() const
{
	FInteractionPayload Payload;
	Payload.DetailData = WeaponInstance;
	Payload.ActionData.ShortActionText = TEXT("줍기");
	Payload.ActionData.LongActionText = TEXT("파괴하기");
	Payload.ActionData.bShowCurrency = true;
	Payload.ActionData.ActionCurrencyType = CachedRewardCurrencyType;
	Payload.ActionData.CurrencyAmount = CachedBaseRewardAmount;
	return Payload;
}

void ACAP_WorldWeapon::DropItem()
{
	if (RootCollision)
	{
		FVector DropImpulse = FVector(0.f,0.f, 600.f);
		RootCollision->AddImpulse(DropImpulse, NAME_None, true);
	}
}

void ACAP_WorldWeapon::OnRootCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                          UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	RootCollision->SetSimulatePhysics(false);
}
