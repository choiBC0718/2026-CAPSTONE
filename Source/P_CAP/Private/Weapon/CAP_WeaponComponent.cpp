// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/CAP_WeaponComponent.h"
#include "Animation/CAP_AnimInstance.h"
#include "CAP_WeaponBase.h"
#include "CAP_WeaponInstance.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/Character.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/CAP_GameplayAbility.h"

UCAP_WeaponComponent::UCAP_WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCAP_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		WeaponMesh_R = NewObject<USkeletalMeshComponent>(OwnerCharacter, TEXT("WeaponMesh_R"));
		WeaponMesh_R->SetupAttachment(OwnerCharacter->GetMesh());
		WeaponMesh_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh_R->ComponentTags.Add("RightHand");
		WeaponMesh_R->RegisterComponent();

		WeaponMesh_L = NewObject<USkeletalMeshComponent>(OwnerCharacter, TEXT("WeaponMesh_L"));
		WeaponMesh_L->SetupAttachment(OwnerCharacter->GetMesh());
		WeaponMesh_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh_L->ComponentTags.Add("LeftHand");
		WeaponMesh_L->RegisterComponent();
	}
	EquippedWeapons.SetNum(2);
	
	if (DefaultBasicWeapon)
	{
		UCAP_WeaponInstance* BasicWeapon = NewObject<UCAP_WeaponInstance>(this);
		BasicWeapon->InitializeWeapon(DefaultBasicWeapon);
		EquippedWeapons[0] = BasicWeapon;
		ApplyWeaponData(BasicWeapon);
	}
	EquippedWeapons[1] = nullptr;
	CurrentWeaponIndex =0;
}

void UCAP_WeaponComponent::PickupWeapon(class UCAP_WeaponInstance* NewWeaponInst)
{
	if (!NewWeaponInst)
		return;

	int32 EmptySlotIndex = INDEX_NONE;
	for (int32 i=0; i<EquippedWeapons.Num() ; ++i)
	{	//빈 무기 슬롯이 있다면 그 슬롯의 인덱스 가져와 
		if (EquippedWeapons[i]==nullptr)
		{
			EmptySlotIndex = i;
			break;
		}
	}

	// 빈 슬롯이 있다면 (현재 무기 하나 사용 중인 경우)
	if (EmptySlotIndex != INDEX_NONE)
	{
		// 슬롯에 바로 장착 및 데이터 적용
		EquippedWeapons[EmptySlotIndex] = NewWeaponInst;
		CurrentWeaponIndex = EmptySlotIndex;
		ApplyWeaponData(NewWeaponInst);
	}
	// 빈 슬롯이 없다면 (현재 무기 두개 사용 중인 경우)
	else
	{
		// 현재 들고있는 무기 땅에 드랍
		UCAP_WeaponInstance* WeaponToDrop = EquippedWeapons[CurrentWeaponIndex];
		if (WeaponToDrop)
		{
			FVector DropLocation = GetOwner()->GetActorLocation();
			FTransform SpawnTransform(FRotator::ZeroRotator, DropLocation);
			
			ACAP_WeaponBase* DroppedWeapon = GetWorld()->SpawnActorDeferred<ACAP_WeaponBase>(ACAP_WeaponBase::StaticClass(), SpawnTransform);
			if (DroppedWeapon)
			{
				DroppedWeapon->WeaponInstance = WeaponToDrop;
				DroppedWeapon->FinishSpawning(SpawnTransform);
				DroppedWeapon->DropWeapon();
			}
		}

		// 무기 슬롯에 상호작용한 무기 장착 및 데이터 적용
		EquippedWeapons[CurrentWeaponIndex] = NewWeaponInst;
		ApplyWeaponData(NewWeaponInst);
	}
}

void UCAP_WeaponComponent::SwapWeapon()
{
	int32 NextIndex = (CurrentWeaponIndex ==0) ? 1:0;
	if (EquippedWeapons[NextIndex] == nullptr)
		return;

	CurrentWeaponIndex = NextIndex;
	ApplyWeaponData(EquippedWeapons[CurrentWeaponIndex]);
}

void UCAP_WeaponComponent::ApplyWeaponData(class UCAP_WeaponInstance* WeaponInstance)
{
	ClearCurrentWeaponData();
	if (!WeaponInstance)
		return;

	UCAP_WeaponDataAsset* WeaponDA = WeaponInstance->GetWeaponDA();
	if (!WeaponDA)
		return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
		return;
	
	// DA에 설정한 메시, 위치로 설정
	for (const FWeaponVisualInfo& VisualInfo :WeaponDA->WeaponVisualInfos)
	{
		USkeletalMeshComponent* TargetMesh = (VisualInfo.EquipHand == EEquipHand::Left) ? WeaponMesh_L : WeaponMesh_R;
		if (TargetMesh)
		{
			// 무기 스켈레탈 메시 설정
			TargetMesh->SetSkeletalMesh(VisualInfo.WeaponMesh.LoadSynchronous());

			// 캐릭터에 부착시킬 뼈 이름
			FName TargetCharacterBone = (VisualInfo.CharacterBoneName != NAME_None) ? VisualInfo.CharacterBoneName : FName("hand_r");

			// 무기 메시를 AttachPoint에 부착 후 위치 조정
			TargetMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetCharacterBone);
			FTransform FinalTransform = VisualInfo.GripOffsetTransform;
			if (VisualInfo.AlignBoneName != NAME_None)
			{
				FTransform BoneTransform = TargetMesh->GetSocketTransform(VisualInfo.AlignBoneName, RTS_Component);
				FinalTransform = BoneTransform.Inverse() * VisualInfo.GripOffsetTransform;
			}
			TargetMesh->SetRelativeTransform(FinalTransform);
		}
	}
	
	if (UCAP_AnimInstance* AnimInst = Cast<UCAP_AnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
	{
		AnimInst->UpdateWeaponAnimData(WeaponDA);
	}

	UCAP_AbilitySystemComponent* ASC = OwnerCharacter->GetComponentByClass<UCAP_AbilitySystemComponent>();
	if (ASC)
	{
		if (WeaponDA->BasicAbility.AbilityClass)
		{
			FGameplayAbilitySpec Spec(WeaponDA->BasicAbility.AbilityClass.LoadSynchronous(), 1, static_cast<int32>(EAbilityInputID::BasicAttack), OwnerCharacter);
			CurrentWeaponAbilityHandles.Add(ASC->GiveAbility(Spec));
		}
		
		int32 SkillInputIndex = static_cast<int32>(EAbilityInputID::Skill1);
		for (const FWeaponSkillData& SkillData : WeaponInstance->GetGrantedSkills())
		{
			if (SkillData.AbilityClass)
			{
				FGameplayAbilitySpec Spec(SkillData.AbilityClass.LoadSynchronous(), 1, SkillInputIndex++, OwnerCharacter);
				CurrentWeaponAbilityHandles.Add(ASC->GiveAbility(Spec));
			}
		}
	}
}

void UCAP_WeaponComponent::ClearCurrentWeaponData()
{
	if (WeaponMesh_R)
		WeaponMesh_R->SetSkeletalMesh(nullptr);
	if (WeaponMesh_L)
		WeaponMesh_L->SetSkeletalMesh(nullptr);
	
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		UCAP_AbilitySystemComponent* ASC = OwnerCharacter->GetComponentByClass<UCAP_AbilitySystemComponent>();
		if (ASC)
		{
			for (const FGameplayAbilitySpecHandle& Handle : CurrentWeaponAbilityHandles)
			{
				ASC->ClearAbility(Handle);
			}
		}
	}
	CurrentWeaponAbilityHandles.Empty();
}

class UCAP_WeaponInstance* UCAP_WeaponComponent::GetCurrentWeaponInstance() const
{
	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		return EquippedWeapons[CurrentWeaponIndex];
	}
	return nullptr;
}



