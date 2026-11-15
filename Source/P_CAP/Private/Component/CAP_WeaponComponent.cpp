// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CAP_WeaponComponent.h"
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
	if (!NewWeaponInst || !NewWeaponInst->GetWeaponDA())
		return;

	// 동일 무기 중복 장착 차단
	if (HasWeapon(NewWeaponInst->GetWeaponDA()))
		return;

	int32 EmptySlotIndex = INDEX_NONE;
	for (int32 i = 0; i < EquippedWeapons.Num(); ++i)
	{
		if (EquippedWeapons[i] == nullptr)
		{
			EmptySlotIndex = i;
			break;
		}
	}

	UCAP_WeaponInstance* WeaponToDrop = nullptr;

	// 빈칸이 없다면 현재 들고 있는 무기를 드랍 (에셋 언로드는 새 무기 로드 완료 후 처리)
	if (EmptySlotIndex == INDEX_NONE)
	{
		EmptySlotIndex = CurrentWeaponIndex;
		WeaponToDrop = EquippedWeapons[EmptySlotIndex];
		if (WeaponToDrop)
		{
			FVector DropLocation = GetOwner()->GetActorLocation() + FVector::UpVector * 10.f;
			FTransform SpawnTransform(FRotator::ZeroRotator, DropLocation);

			ACAP_WorldWeapon* DroppedWeapon = GetWorld()->SpawnActorDeferred<ACAP_WorldWeapon>(ACAP_WorldWeapon::StaticClass(), SpawnTransform);
			if (DroppedWeapon)
			{
				DroppedWeapon->WeaponInstance = WeaponToDrop;
				DroppedWeapon->FinishSpawning(SpawnTransform);
				DroppedWeapon->DropItem();
			}
		}
	}

	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		DeactivateAndClearWeapon(EquippedWeapons[CurrentWeaponIndex]);
	}

	EquippedWeapons[EmptySlotIndex] = NewWeaponInst;
	CurrentWeaponIndex = EmptySlotIndex;

	NewWeaponInst->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(this, [this, NewWeaponInst, WeaponToDrop]()
	{
		if (WeaponToDrop)
		{
			WeaponToDrop->UnloadWeaponAssets();
		}

		// 비동기 로딩 도중 다른 슬롯으로 스왑한 경우 덮어쓰기 방지
		if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex) && EquippedWeapons[CurrentWeaponIndex] == NewWeaponInst)
		{
			ApplyWeaponData(NewWeaponInst);
		}
	}));
}

void UCAP_WeaponComponent::SwapWeapon()
{
	int32 NextIndex = (CurrentWeaponIndex == 0) ? 1 : 0;
	if (!EquippedWeapons.IsValidIndex(NextIndex) || EquippedWeapons[NextIndex] == nullptr)
		return;

	UCAP_WeaponInstance* OldWeapon = EquippedWeapons.IsValidIndex(CurrentWeaponIndex) ? EquippedWeapons[CurrentWeaponIndex] : nullptr;
	UCAP_WeaponInstance* NextWeapon = EquippedWeapons[NextIndex];

	DeactivateAndClearWeapon(OldWeapon);

	CurrentWeaponIndex = NextIndex;

	// 다음 무기의 에셋이 메모리에 로드된 상태임을 보장한 뒤 메시/ABP/어빌리티 적용
	NextWeapon->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(this, [this, NextWeapon]()
	{
		if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex) && EquippedWeapons[CurrentWeaponIndex] == NextWeapon)
		{
			ApplyWeaponData(NextWeapon);
		}
	}));
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


void UCAP_WeaponComponent::ApplyWeaponData(class UCAP_WeaponInstance* WeaponInstance)
{
	if (!WeaponInstance || !WeaponInstance->GetWeaponDA())
		return;

	UCAP_WeaponDataAsset* WeaponDA = WeaponInstance->GetWeaponDA();
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
		return;
	
	// 캐릭터 스켈레탈 메쉬 스왑
	if (USkeletalMesh* LoadedCharMesh = WeaponDA->CharacterMesh.Get())
	{
		OwnerCharacter->GetMesh()->SetSkeletalMesh(LoadedCharMesh);
	}

	// 캐릭터 전용 애니메이션 블루프린트 스왑
	if (UClass* LoadedAnimClass = WeaponDA->CharacterAnimClass.Get())
	{
		OwnerCharacter->GetMesh()->SetAnimInstanceClass(LoadedAnimClass);
	}
	
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

		BasicWeapon->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(this, [this, BasicWeapon]()
		{
			if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex) && EquippedWeapons[CurrentWeaponIndex] == BasicWeapon)
			{
				ApplyWeaponData(BasicWeapon);
			}
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
	for (int32 i = 0; i < EquippedWeapons.Num(); ++i)
	{
		if (EquippedWeapons[i])
		{
			DeactivateAndClearWeapon(EquippedWeapons[i]);
			EquippedWeapons[i]->UnloadWeaponAssets();
			EquippedWeapons[i] = nullptr;
		}
	}

	for (int32 i = 0; i < InData.HeldWeapons.Num() && i < MaxWeaponCount; ++i)
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

	CurrentWeaponIndex = FMath::Clamp(InData.EquippedWeaponIdx, 0, MaxWeaponCount - 1);
	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex) && EquippedWeapons[CurrentWeaponIndex])
	{
		UCAP_WeaponInstance* ActiveWeapon = EquippedWeapons[CurrentWeaponIndex];
		ActiveWeapon->LoadWeaponAssets(FStreamableDelegate::CreateWeakLambda(this, [this, ActiveWeapon]()
		{
			if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex) && EquippedWeapons[CurrentWeaponIndex] == ActiveWeapon)
			{
				ApplyWeaponData(ActiveWeapon);
			}
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

void UCAP_WeaponComponent::DeactivateAndClearWeapon(class UCAP_WeaponInstance* WeaponInst)
{
	if (!WeaponInst)
		return;

	if (ASC)
	{
		// 1. 타겟팅 어빌리티(AoE 등)가 대기 중이면 먼저 취소하여 TargetActor/인디케이터 정리
		ASC->TargetCancel();

		// 2. ClearAbility 전에 CancelAbilityHandle을 먼저 호출해야 EndAbility가 실행되어
		//    BlockAbilitiesWithTag(BasicAttackTag) 및 콜리전 무시 상태가 정상 복구됨
		TArray<FGameplayAbilitySpecHandle> HandlesCopy = WeaponInst->GrantedAbilityHandles;
		for (const FGameplayAbilitySpecHandle& Handle : HandlesCopy)
		{
			if (Handle.IsValid())
			{
				ASC->CancelAbilityHandle(Handle);
				ASC->ClearAbility(Handle);
			}
		}
		WeaponInst->GrantedAbilityHandles.Empty();
	}

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (OwnerCharacter->GetMesh())
		{
			if (UAnimInstance* AnimInst = OwnerCharacter->GetMesh()->GetAnimInstance())
			{
				AnimInst->Montage_Stop(0.0f, nullptr);
			}
		}
	}
}
