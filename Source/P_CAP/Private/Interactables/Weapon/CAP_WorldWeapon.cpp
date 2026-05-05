// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactables/Weapon/CAP_WorldWeapon.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "CAP_WeaponInstance.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_WeaponComponent.h"
#include "Components/SphereComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GAS/Setting/CAP_AttributeSet.h"

ACAP_WorldWeapon::ACAP_WorldWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	RootCollision = CreateDefaultSubobject<USphereComponent>("RootCollision");
	SetRootComponent(RootCollision);
	
	MeshContainer=CreateDefaultSubobject<USceneComponent>("MeshContainer");
	MeshContainer->SetupAttachment(InteractionSphere);
	
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
	RootCollision->OnComponentHit.AddDynamic(this, &ACAP_WorldWeapon::OnRootCollisionHit);
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
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Player);
		UCAP_AbilitySystemComponent* P_ASC = Cast<UCAP_AbilitySystemComponent>(ASC);
		UCAP_CurrencyComponent* CurrencyComp = Player->GetCurrencyComponent();
		if (!ASC || !P_ASC || !CurrencyComp)
			return;

		const UCAP_AbilitySystemGenerics* Generics = P_ASC->GetGenerics();
		if (Generics && Generics->GetDisassembleRewardDataTable())
		{
			EItemGrade TargetGrade = WeaponInstance->GetWeaponDA() ? WeaponInstance->GetWeaponDA()->ItemGrade : EItemGrade::Normal;
			
			FString EnumString = UEnum::GetValueAsString(TargetGrade);
			FString GradeName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2); 
				
			FDisassembleRewardRow* Row = Generics->GetDisassembleRewardDataTable()->FindRow<FDisassembleRewardRow>(FName(*GradeName), "");
			if (Row)
			{
				float BonusMultiplier = ASC->GetNumericAttribute(UCAP_AttributeSet::GetDisassembleBonusMultiplierAttribute());
				int32 FinalAmount = FMath::RoundToInt(Row->WeaponRewardAmount * (1.0f + BonusMultiplier));
					
				CurrencyComp->AddCurrency(Row->WeaponCurrencyType, FinalAmount);
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
	Payload.ActionData.ActionCurrencyType = ECurrencyType::WeaponMaterial;
	Payload.ActionData.CurrencyAmount = 5; 
	return Payload;
}

void ACAP_WorldWeapon::OnRootCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	RootCollision->SetSimulatePhysics(false);
}
