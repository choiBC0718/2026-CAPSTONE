// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon/CAP_WeaponComponent.h"
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
	EquippedWeapons.SetNum(MaxWeaponCount);
	
	if (DefaultBasicWeapon)
	{
		UCAP_WeaponInstance* BasicWeapon = NewObject<UCAP_WeaponInstance>(this);
		BasicWeapon->InitializeWeapon(DefaultBasicWeapon);
		EquippedWeapons[0] = BasicWeapon;
		
		BasicWeapon->LoadWeaponAssets(FStreamableDelegate::CreateLambda([this, BasicWeapon]()
		{
			ApplyWeaponData(BasicWeapon);
		}));
	}
	EquippedWeapons[1] = nullptr;
	CurrentWeaponIndex =0;

	ASC = OwnerCharacter->GetComponentByClass<UCAP_AbilitySystemComponent>();
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
				DroppedWeapon->DropItem();
			}
			WeaponToDrop->UnloadWeaponAssets();
			RemoveWeaponAbilities(WeaponToDrop);
		}

		// 무기 슬롯에 상호작용한 무기 장착 및 데이터 적용
		EquippedWeapons[CurrentWeaponIndex] = NewWeaponInst;
	}

	// 즉시 ApplyWeaponData가 아닌, 메모리에 로딩 후에 Apply ** 무기 획득이 처음인 경우에만 해당 (무기 교체시엔 적용 x)
	NewWeaponInst->LoadWeaponAssets(FStreamableDelegate::CreateLambda([this, NewWeaponInst]()
	{
		ApplyWeaponData(NewWeaponInst);
	}));
}

void UCAP_WeaponComponent::SwapWeapon()
{
	int32 NextIndex = (CurrentWeaponIndex ==0) ? 1:0;
	if (EquippedWeapons[NextIndex] == nullptr)
		return;
	// 교체된 무기 능력 언마운트
	UnmapWeaponAbilities(EquippedWeapons[CurrentWeaponIndex]);
	if (ASC)
		ASC->CancelAllAbilities();

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			// BlendOut 시간을 0.0f로 주어 몽타주를 1프레임 만에 즉시 꺼버립니다!
			AnimInst->Montage_Stop(0.0f, nullptr);
		}
	}
	
	CurrentWeaponIndex = NextIndex;
	ApplyWeaponData(EquippedWeapons[CurrentWeaponIndex]);
}

class USkeletalMeshComponent* UCAP_WeaponComponent::GetWeaponMesh(EEquipHand Hand) const
{
	return Hand == EEquipHand::Left ? WeaponMesh_L : WeaponMesh_R; 
}

void UCAP_WeaponComponent::ApplyWeaponData(class UCAP_WeaponInstance* WeaponInstance)
{
	ClearCurrentWeaponVisuals();
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
			TargetMesh->SetSkeletalMesh(VisualInfo.WeaponMesh.Get());
			TargetMesh->EmptyOverrideMaterials();
			
			FTransform FinalTransform = VisualInfo.GripOffsetTransform;
			if (VisualInfo.AlignBoneName != NAME_None)
			{
				FTransform BoneTransform = TargetMesh->GetSocketTransform(VisualInfo.AlignBoneName, RTS_Component);
				FinalTransform = BoneTransform.Inverse() * VisualInfo.GripOffsetTransform;
			}
			
			// 캐릭터에 부착시킬 뼈 이름
			FName TargetCharacterBone = (VisualInfo.CharacterBoneName != NAME_None) ? VisualInfo.CharacterBoneName : FName("hand_r");
			// 무기 메시를 AttachPoint에 부착 후 위치 조정
			TargetMesh->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetCharacterBone);
			TargetMesh->SetRelativeTransform(FinalTransform);
		}
	}
	
	if (UCAP_AnimInstance* AnimInst = Cast<UCAP_AnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
	{
		AnimInst->UpdateWeaponAnimData(WeaponDA);
	}

	GrantWeaponAbilities(WeaponInstance);
	MapWeaponAbilities(WeaponInstance);

	if (OnWeaponChanged.IsBound())
	{
		int32 StandbyIdx = (CurrentWeaponIndex ==0) ? 1:0;
		UCAP_WeaponInstance* StandbyWeapon = nullptr;
		if (EquippedWeapons.IsValidIndex(StandbyIdx))
			StandbyWeapon = EquippedWeapons[StandbyIdx];
		
		// 현재 변경된 무기 인스턴스를 방송
		OnWeaponChanged.Broadcast(WeaponInstance, StandbyWeapon);
	}
}

void UCAP_WeaponComponent::ClearCurrentWeaponVisuals()
{
	if (WeaponMesh_R)
		WeaponMesh_R->SetSkeletalMesh(nullptr);
	if (WeaponMesh_L)
		WeaponMesh_L->SetSkeletalMesh(nullptr);
}

void UCAP_WeaponComponent::GrantWeaponAbilities(class UCAP_WeaponInstance* WeaponInst)
{
	if (!ASC || !WeaponInst || WeaponInst->GrantedAbilityHandles.Num() > 0)
		return;
	// WeaponInstance에 부여받은 능력을 추가하기
	const FWeaponSkillData* BasicAttack = WeaponInst->GetBasicAttack();
	if (BasicAttack && BasicAttack->AbilityClass.Get())
	{
		FGameplayAbilitySpec Spec(BasicAttack->AbilityClass.Get(), 1, INDEX_NONE, GetOwner());
		WeaponInst->GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
	}
	
	for (const FWeaponSkillData& SkillData : WeaponInst->GetGrantedSkills())
	{
		if (SkillData.AbilityClass.Get())
		{
			FGameplayAbilitySpec Spec(SkillData.AbilityClass.Get(), 1, INDEX_NONE, GetOwner());
			WeaponInst->GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
		}
	}
}

void UCAP_WeaponComponent::MapWeaponAbilities(class UCAP_WeaponInstance* WeaponInst)
{
	if (!ASC || !WeaponInst)
		return;

	int32 HandleIndex = 0;
	// HandleInex ( 0 = BasicAttack / 1 = Skill1 / 2 = Skill2 )
	if (WeaponInst->GetBasicAttack() && WeaponInst->GetBasicAttack()->AbilityClass.Get())
	{
		if (WeaponInst->GrantedAbilityHandles.IsValidIndex(HandleIndex))
		{
			FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(WeaponInst->GrantedAbilityHandles[HandleIndex]);
			if (Spec) Spec->InputID = static_cast<int32>(EAbilityInputID::BasicAttack);
			HandleIndex++;
		}
	}

	int32 SkillInputIndex = static_cast<int32>(EAbilityInputID::Skill1);
	for (const FWeaponSkillData& SkillData : WeaponInst->GetGrantedSkills())
	{
		if (SkillData.AbilityClass.Get() && WeaponInst->GrantedAbilityHandles.IsValidIndex(HandleIndex))
		{
			FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(WeaponInst->GrantedAbilityHandles[HandleIndex]);
			if (Spec) Spec->InputID = SkillInputIndex++;
			HandleIndex++;
		}
	}
}

void UCAP_WeaponComponent::UnmapWeaponAbilities(class UCAP_WeaponInstance* WeaponInst)
{
	if (!ASC || !WeaponInst)
		return;

	for (const FGameplayAbilitySpecHandle& Handle : WeaponInst->GrantedAbilityHandles)
	{
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Handle);
		if (Spec)
		{
			Spec->InputID = INDEX_NONE;
		}
	}
}

void UCAP_WeaponComponent::RemoveWeaponAbilities(class UCAP_WeaponInstance* WeaponInst)
{
	if (!ASC || !WeaponInst) return;

	for (const FGameplayAbilitySpecHandle& Handle : WeaponInst->GrantedAbilityHandles)
	{
		ASC->ClearAbility(Handle);
	}
	WeaponInst->GrantedAbilityHandles.Empty();
}

class UCAP_WeaponInstance* UCAP_WeaponComponent::GetCurrentWeaponInstance() const
{
	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		return EquippedWeapons[CurrentWeaponIndex];
	}
	return nullptr;
}



