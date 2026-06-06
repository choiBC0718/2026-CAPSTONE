// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_WeaponComponent.h"
#include "Animation/CAP_AnimInstance.h"
#include "Interactables/Weapon/CAP_WorldWeapon.h"
#include "Interactables/Weapon/CAP_WeaponInstance.h"
#include "GameplayAbilitySpec.h"
#include "GameFramework/Character.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "GAS/Ability/Flow/GA_FlowBase.h"
#include "GAS/Ability/Payload/GA_PayloadBase.h"
#include "Kismet/GameplayStatics.h"

UCAP_WeaponComponent::UCAP_WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCAP_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeWeaponMeshes();

	MaxWeaponCount = FMath::Max(1, MaxWeaponCount);
	EquippedWeapons.SetNum(MaxWeaponCount);
	CurrentWeaponIndex =0;
	
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
		ASC=OwnerChar->GetComponentByClass<UCAP_AbilitySystemComponent>();

	if (!TryRestoreSavedWeapons())
		EquipDefaultWeapon();
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
	// 빈칸 없다면 현재 들고있는 무기 버림
	if (EmptySlotIndex == INDEX_NONE)
	{
		EmptySlotIndex = CurrentWeaponIndex;
		UCAP_WeaponInstance* WeaponToDrop = EquippedWeapons[EmptySlotIndex];
		if (WeaponToDrop)
		{
			FVector DropLocation = GetOwner()->GetActorLocation() + FVector::UpVector*10.f;
			FTransform SpawnTransform(FRotator::ZeroRotator, DropLocation);

			ACAP_WorldWeapon* DroppedWeapon = GetWorld()->SpawnActorDeferred<ACAP_WorldWeapon>(ACAP_WorldWeapon::StaticClass(), SpawnTransform);
			if (DroppedWeapon)
			{
				DroppedWeapon->WeaponInstance = WeaponToDrop;
				DroppedWeapon->FinishSpawning(SpawnTransform);
				DroppedWeapon->DropItem();
			}
			WeaponToDrop->UnloadWeaponAssets();
		}
	}
	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex))
		ClearAbilities(EquippedWeapons[CurrentWeaponIndex]);
	// 무기 장착
	EquippedWeapons[EmptySlotIndex] = NewWeaponInst;
	CurrentWeaponIndex = EmptySlotIndex;
	
	// 즉시 ApplyWeaponData가 아닌, 메모리에 로딩 후에 Apply ** 무기 획득이 처음인 경우에만 해당 (무기 교체시엔 적용 x)
	NewWeaponInst->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(this,[this, NewWeaponInst]()
	{
		ApplyWeaponData(NewWeaponInst);
	}));
}

void UCAP_WeaponComponent::SwapWeapon()
{
	int32 NextIndex = (CurrentWeaponIndex ==0) ? 1:0;
	if (EquippedWeapons[NextIndex] == nullptr)
		return;
	ClearAbilities(EquippedWeapons[CurrentWeaponIndex]);

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
		{
			AnimInst->Montage_Stop(0.0f, nullptr);
		}
	}
	
	CurrentWeaponIndex = NextIndex;
	ApplyWeaponData(EquippedWeapons[CurrentWeaponIndex]);
}

bool UCAP_WeaponComponent::HasWeapon(class UCAP_WeaponDataAsset* WeaponDA) const
{
	for (UCAP_WeaponInstance* WeaponInst : EquippedWeapons)
	{
		if (WeaponInst && WeaponInst->GetWeaponDA() == WeaponDA)
			return true;
	}
	return false;
}

class USkeletalMeshComponent* UCAP_WeaponComponent::GetWeaponMesh(EEquipHand Hand) const
{
	return Hand == EEquipHand::Left ? WeaponMesh_L : WeaponMesh_R; 
}

void UCAP_WeaponComponent::ApplyWeaponData(class UCAP_WeaponInstance* WeaponInstance)
{
	if (!WeaponInstance)
		return;

	UCAP_WeaponDataAsset* WeaponDA = WeaponInstance->GetWeaponDA();
	if (!WeaponDA)
		return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
		return;
	
	if (UCAP_AnimInstance* AnimInst = Cast<UCAP_AnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
	{
		AnimInst->UpdateWeaponAnimData(WeaponDA);
	}
	AttachWeaponMesh(WeaponDA);
	SetDodgeAbility(WeaponDA);
	GrantAbilities(WeaponInstance);

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

void UCAP_WeaponComponent::AttachWeaponMesh(class UCAP_WeaponDataAsset* WeaponDA)
{
	if (!WeaponDA)
		return;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
		return;
	
	if (WeaponMesh_R)
		WeaponMesh_R->SetSkeletalMesh(nullptr);
	if (WeaponMesh_L)
		WeaponMesh_L->SetSkeletalMesh(nullptr);
	
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
}

void UCAP_WeaponComponent::InitializeWeaponMeshes()
{
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
}

bool UCAP_WeaponComponent::TryRestoreSavedWeapons()
{
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(this))
	{
		if (UCAP_ProgressionSubsystem* Subsys = GI->GetSubsystem<UCAP_ProgressionSubsystem>())
		{
			FPlayerProgressionData Data;
			if (Subsys->LoadPlayerProgression(Data) && Data.bIsValid)
			{
				RestoreFromSaveData(Data.WeaponData);
				return true;
			}
		}
	}
	return false;
}

void UCAP_WeaponComponent::EquipDefaultWeapon()
{
	if (DefaultBasicWeapon)
	{
		UCAP_WeaponInstance* BasicWeapon = NewObject<UCAP_WeaponInstance>(this);
		BasicWeapon->Initialize(DefaultBasicWeapon);
		EquippedWeapons[0] = BasicWeapon;
		
		BasicWeapon->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(this,[this, BasicWeapon]()
		{
			ApplyWeaponData(BasicWeapon);
		}));
	}
}

void UCAP_WeaponComponent::GrantAbilities(class UCAP_WeaponInstance* WeaponInst)
{
	if (!ASC || !WeaponInst || WeaponInst->GrantedAbilityHandles.Num() > 0)
		return;

	const int32 PAYLOAD_INPUT_OFFSET = 100;

	// WeaponInstance에 부여받은 능력을 추가하기
	auto GrantSkillFunc = [&](const FWeaponSkillData* SkillData, int32 InputID)
	{
		if (!SkillData)
			return;
		if (SkillData->InputAbilityClass)
		{
			FGameplayAbilitySpec Spec(SkillData->InputAbilityClass, 1, InputID, WeaponInst);
			WeaponInst->GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
		}
		for (TSubclassOf<UGA_PayloadBase> PayloadClass : SkillData->PayloadAbilityClass)
		{
			if (PayloadClass)
			{
				FGameplayAbilitySpec Spec(PayloadClass, 1, InputID+PAYLOAD_INPUT_OFFSET, WeaponInst);
				WeaponInst->GrantedAbilityHandles.Add(ASC->GiveAbility(Spec));
			}
		}
	};
	
	GrantSkillFunc(WeaponInst->GetBasicAttack(), static_cast<int32>(EAbilityInputID::BasicAttack));
	
	int32 SkillInputIndex = static_cast<int32>(EAbilityInputID::Skill1);
	for (const FWeaponSkillData& Skill : WeaponInst->GetGrantedSkills())
	{
		GrantSkillFunc(&Skill, SkillInputIndex);
		SkillInputIndex++;
	}
}

void UCAP_WeaponComponent::ClearAbilities(class UCAP_WeaponInstance* WeaponInst)
{
	if (!ASC || !WeaponInst) return;
	
	for (const FGameplayAbilitySpecHandle& Handle : WeaponInst->GrantedAbilityHandles)
	{
		ASC->ClearAbility(Handle);
	}
	WeaponInst->GrantedAbilityHandles.Empty();
}

struct FWeaponComponentSaveData UCAP_WeaponComponent::CreateSaveData() const
{
	FWeaponComponentSaveData SaveData;
	SaveData.EquippedWeaponIdx = CurrentWeaponIndex;
	
	for (int32 i=0 ;i<EquippedWeapons.Num();++i)
	{
		if (EquippedWeapons[i])
			SaveData.HeldWeapons.Add(EquippedWeapons[i]->CreateSaveData());
		else
			SaveData.HeldWeapons.Add(FWeaponSaveData());
	}
	return SaveData;
}

void UCAP_WeaponComponent::RestoreFromSaveData(const struct FWeaponComponentSaveData& InData)
{
	// 기존 무기 메모리 해제 & 능력 제거
	for (int32 i=0; i<EquippedWeapons.Num(); ++i)
	{
		if (EquippedWeapons[i])
		{
			ClearAbilities(EquippedWeapons[i]);
			EquippedWeapons[i]->UnloadWeaponAssets();
			EquippedWeapons[i]=nullptr;
		}
	}

	// 무기 데이터 백업
	for (int32 i=0;i<InData.HeldWeapons.Num() && i<MaxWeaponCount ; ++i)
	{
		const FWeaponSaveData& Data = InData.HeldWeapons[i];
		if (Data.WeaponDA)
		{
			UCAP_WeaponInstance* NewWeapon = NewObject<UCAP_WeaponInstance>(this);
			NewWeapon->Initialize(Data.WeaponDA);
			NewWeapon->RestoreFromSaveData(Data);
			EquippedWeapons[i] = NewWeapon;
		}
	}
	// 들고있는 무기 백업
	CurrentWeaponIndex = FMath::Clamp(InData.EquippedWeaponIdx,0,MaxWeaponCount-1);
	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex) && EquippedWeapons[CurrentWeaponIndex])
	{
		UCAP_WeaponInstance* ActiveWeapon = EquippedWeapons[CurrentWeaponIndex];
		ActiveWeapon->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(this, [this, ActiveWeapon]()
		{	// 메쉬 & 능력 부여
			ApplyWeaponData(ActiveWeapon);
		}));
	}
}

class UCAP_WeaponInstance* UCAP_WeaponComponent::GetCurrentWeaponInstance() const
{
	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		return EquippedWeapons[CurrentWeaponIndex];
	}
	return nullptr;
}

void UCAP_WeaponComponent::SetDodgeAbility(class UCAP_WeaponDataAsset* WeaponDA)
{
	if (!WeaponDA)
		return;
	MaxDodgeCount = WeaponDA->MaxDodgeCount;
	CurrentDodgeCount = MaxDodgeCount;

	GetWorld()->GetTimerManager().ClearTimer(ComboTimer);
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);
}

void UCAP_WeaponComponent::ConsumeDodge()
{
	if (CurrentDodgeCount <= 0)	return;
	CurrentDodgeCount--;
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimer);
	GetWorld()->GetTimerManager().ClearTimer(ComboTimer);
	if (CurrentDodgeCount <=0)
		GetWorld()->GetTimerManager().SetTimer(CooldownTimer,this, &UCAP_WeaponComponent::OnCooldownFinished, DodgeCooldown,false);
	else
		GetWorld()->GetTimerManager().SetTimer(ComboTimer,this, &UCAP_WeaponComponent::OnComboWindowExpired, ComboWindowTime,false);
}

void UCAP_WeaponComponent::OnComboWindowExpired()
{
	CurrentDodgeCount=0;
	GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UCAP_WeaponComponent::OnCooldownFinished, DodgeCooldown,false);
}

void UCAP_WeaponComponent::OnCooldownFinished()
{
	CurrentDodgeCount = MaxDodgeCount;
}
