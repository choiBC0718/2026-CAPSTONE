// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerCharacter.h"

#include "CAP_PlayerController.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Animation/CAP_AnimInstance.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "Interface/CAP_InteractInterface.h"
#include "Weapon/CAP_WeaponBase.h"
#include "Weapon/CAP_WeaponInstance.h"

ACAP_PlayerCharacter::ACAP_PlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm Comp");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritYaw = false;    
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera Comp");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	WeaponAttachPoint_R = CreateDefaultSubobject<USceneComponent>("Weapon Attach Point R");
	WeaponAttachPoint_R->SetupAttachment(GetMesh(), "hand_r");
	
	WeaponMesh_R = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh_R");
	WeaponMesh_R->SetupAttachment(WeaponAttachPoint_R);
	WeaponMesh_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh_R->ComponentTags.Add("RightHand");

	WeaponAttachPoint_L = CreateDefaultSubobject<USceneComponent>("Weapon Attach Point L");
	WeaponAttachPoint_L->SetupAttachment(GetMesh(), "hand_l");
	
	WeaponMesh_L = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Skeletal Mesh_L");
	WeaponMesh_L->SetupAttachment(WeaponAttachPoint_L);
	WeaponMesh_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh_L->ComponentTags.Add("LeftHand");
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ACAP_PlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
		
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(GameplayIMC);
			InputSubsystem->AddMappingContext(GameplayIMC, 0);
		}
	}
}

void ACAP_PlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC)
	{
		ASC->InitAbilityActorInfo(this,this);
		ASC->InitComponent(CharacterStatRowName);
	}
}

void ACAP_PlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(MoveIA, ETriggerEvent::Triggered, this, &ACAP_PlayerCharacter::MoveInputHandle);
		EnhancedInputComp->BindAction(MoveIA, ETriggerEvent::Completed, this, &ACAP_PlayerCharacter::MoveInputHandle);
		EnhancedInputComp->BindAction(MoveIA, ETriggerEvent::Canceled, this, &ACAP_PlayerCharacter::MoveInputHandle);
		EnhancedInputComp->BindAction(SwapIA, ETriggerEvent::Started, this, &ACAP_PlayerCharacter::SwapWeapon);
		
		EnhancedInputComp->BindAction(InteractIA, ETriggerEvent::Ongoing, this, &ACAP_PlayerCharacter::InteractInputHandle);
		EnhancedInputComp->BindAction(InteractIA, ETriggerEvent::Triggered, this, &ACAP_PlayerCharacter::InteractInputHandle);
		EnhancedInputComp->BindAction(InteractIA, ETriggerEvent::Completed, this, &ACAP_PlayerCharacter::InteractInputHandle);
		EnhancedInputComp->BindAction(InteractIA, ETriggerEvent::Canceled, this, &ACAP_PlayerCharacter::InteractInputHandle);
		
		for (const TPair<EAbilityInputID, UInputAction*> InputActionPair : AbilityInputActions)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Started, this, &ACAP_PlayerCharacter::AbilityInputHandle, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Completed, this, &ACAP_PlayerCharacter::AbilityInputHandle, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Canceled, this, &ACAP_PlayerCharacter::AbilityInputHandle, InputActionPair.Key);
		}
	}
}

void ACAP_PlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	EquippedWeapons.SetNum(2);

	//TODO 스테이지 넘어갈 때 기본무기 장착하는 로직 제외
	if (DefaultBasicWeapon)
	{
		UCAP_WeaponInstance* BasicWeapon = NewObject<UCAP_WeaponInstance>(this);
		BasicWeapon->InitializeWeapon(DefaultBasicWeapon);

		EquippedWeapons[0] = BasicWeapon;
		ApplyWeaponData(BasicWeapon);
	}
	EquippedWeapons[1] = nullptr;
	CurrentWeaponIndex = 0;
}

void ACAP_PlayerCharacter::PickupWeapon(class UCAP_WeaponInstance* NewWeaponInstance)
{
	if (!NewWeaponInstance)
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
		EquippedWeapons[EmptySlotIndex] = NewWeaponInstance;
		CurrentWeaponIndex = EmptySlotIndex;
		ApplyWeaponData(NewWeaponInstance);
	}
	// 빈 슬롯이 없다면 (현재 무기 두개 사용 중인 경우)
	else
	{
		// 현재 들고있는 무기 땅에 드랍
		UCAP_WeaponInstance* WeaponToDrop = EquippedWeapons[CurrentWeaponIndex];
		if (WeaponToDrop)
		{
			FVector DropLocation = GetActorLocation() + (GetActorForwardVector() * 50.f);
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			
			ACAP_WeaponBase* DroppedWeapon = GetWorld()->SpawnActor<ACAP_WeaponBase>(DropLocation, FRotator::ZeroRotator, SpawnParams);
			if (DroppedWeapon)
			{
				// 버린 무기의 Instance 주입 및 스태틱 메시 설정(OnConstruction) 
				DroppedWeapon->WeaponInstance = WeaponToDrop;
				DroppedWeapon->OnConstruction(DroppedWeapon->GetTransform());
			}
		}

		// 무기 슬롯에 상호작용한 무기 장착 및 데이터 적용
		EquippedWeapons[CurrentWeaponIndex] = NewWeaponInstance;
		ApplyWeaponData(NewWeaponInstance);
	}
}

class UCAP_WeaponInstance* ACAP_PlayerCharacter::GetCurrentWeaponInstance() const
{
	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		return EquippedWeapons[CurrentWeaponIndex];
	}
	return nullptr;
}

void ACAP_PlayerCharacter::UpdateInteractUI(bool bVisible)
{
	ACAP_PlayerController* PC = Cast<ACAP_PlayerController>(GetController());
	if (!PC)
		return;

	FString CurrentKeyName = "F";
	if (bVisible && InteractIA)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			TArray<FKey> MappedKeys = Subsystem->QueryKeysMappedToAction(InteractIA);
			if (MappedKeys.Num() > 0)
			{
				// IMC에서 설정한 키 텍스트 가져오기
				CurrentKeyName = MappedKeys[0].GetDisplayName().ToString();
			}
		}
	}
	PC->SetInteractUIVisibility(bVisible, CurrentKeyName);
}

void ACAP_PlayerCharacter::UpdateInteractProgress(float Progress)
{
	ACAP_PlayerController* PC = Cast<ACAP_PlayerController>(GetController());
	if (PC)
	{
		PC->UpdateInteractProgressUI(Progress);
	}
}

FVector ACAP_PlayerCharacter::GetMoveForwardDir()
{
	FVector Forward = Camera->GetForwardVector();
	Forward.Z = 0;
	Forward.Normalize();
	return Forward;
}

FVector ACAP_PlayerCharacter::GetMoveRightDir()
{
	FVector Right = Camera->GetRightVector();
	Right.Z = 0;
	Right.Normalize();
	return Right;
}

void ACAP_PlayerCharacter::MoveInputHandle(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal = InputActionValue.Get<FVector2D>();
	if (InputVal.IsNearlyZero()) return;

	InputVal.Normalize();

	AddMovementInput(GetMoveForwardDir() * InputVal.Y + GetMoveRightDir() * InputVal.X);
}

void ACAP_PlayerCharacter::AbilityInputHandle(const FInputActionValue& InputActionValue, EAbilityInputID AbilityInputID)
{
	bool bPressed = InputActionValue.Get<bool>();
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)AbilityInputID);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)AbilityInputID);
	}
}

void ACAP_PlayerCharacter::InteractInputHandle(const FInputActionInstance& Instance)
{
	ETriggerEvent TriggerEvent = Instance.GetTriggerEvent();
	float ElapsedTime = Instance.GetElapsedTime();
	float HoldDuration = 1.f;
	
	if (TriggerEvent == ETriggerEvent::Ongoing)
	{
		float Progress = FMath::Clamp(ElapsedTime / HoldDuration, 0.0f, 1.0f);
		UpdateInteractProgress(Progress);
	}
	else if (TriggerEvent == ETriggerEvent::Triggered)
	{
		if (InteractableActor)
		{
			ICAP_InteractInterface* InteractableObj = Cast<ICAP_InteractInterface>(InteractableActor);
			if (InteractableObj)
			{
				InteractableObj->InteractDisassemble(this);
			}
			UpdateInteractProgress(0.f);
		}
	}
	else if (TriggerEvent == ETriggerEvent::Completed || TriggerEvent == ETriggerEvent::Canceled)
	{
		if (ElapsedTime < 0.5f && InteractableActor)
		{
			ICAP_InteractInterface* InteractableObj = Cast<ICAP_InteractInterface>(InteractableActor);
			if (InteractableObj)
			{
				InteractableObj->InteractEquip(this);
			}
		}
		UpdateInteractProgress(0.f);
	}
}

void ACAP_PlayerCharacter::SwapWeapon()
{
	int32 NextIndex = (CurrentWeaponIndex ==0) ? 1:0;
	if (EquippedWeapons[NextIndex] == nullptr)
		return;

	CurrentWeaponIndex = NextIndex;
	ApplyWeaponData(EquippedWeapons[CurrentWeaponIndex]);
}

void ACAP_PlayerCharacter::ApplyWeaponData(class UCAP_WeaponInstance* WeaponInstance)
{
	ClearCurrentWeaponData();
	if (!WeaponInstance)
		return;

	UCAP_WeaponDataAsset* WeaponDA = WeaponInstance->GetWeaponDA();
	if (!WeaponDA)
		return;
	
	// DA에 설정한 메시, 위치로 설정
	for (const FWeaponVisualInfo& VisualInfo :WeaponDA->WeaponVisualInfos)
	{
		USkeletalMeshComponent* TargetMesh = (VisualInfo.EquipHand == EEquipHand::Left) ? WeaponMesh_L : WeaponMesh_R;
		USceneComponent* AttachPoint = (VisualInfo.EquipHand == EEquipHand::Left) ? WeaponAttachPoint_L : WeaponAttachPoint_R;
		if (TargetMesh && AttachPoint)
		{
			// 무기 스켈레탈 메시 설정
			TargetMesh->SetSkeletalMesh(VisualInfo.WeaponMesh);

			// DA에 설정한 무기를 부착시킬 본(소켓) 이름의 위치로 SceneComponent 위치 수정
			FName TargetCharacterBone = (VisualInfo.CharacterBoneName != NAME_None) ? VisualInfo.CharacterBoneName : FName("hand_r");
			AttachPoint->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, TargetCharacterBone);
			AttachPoint->SetRelativeTransform(VisualInfo.GripOffsetTransform);

			// 무기 메시를 AttachPoint에 부착 후 위치 조정
			TargetMesh->AttachToComponent(AttachPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			if (VisualInfo.AlignBoneName != NAME_None)
			{
				FTransform BoneTransform = TargetMesh->GetSocketTransform(VisualInfo.AlignBoneName, RTS_Component);
				TargetMesh->SetRelativeTransform(BoneTransform.Inverse());
			}
			else
			{
				TargetMesh->SetRelativeTransform(FTransform::Identity);
			}
		}
	}
	
	if (UCAP_AnimInstance* AnimInst = Cast<UCAP_AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->UpdateWeaponAnimData(WeaponDA);
	}

	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC)
	{
		if (WeaponDA->BasicAbility.AbilityClass)
		{
			FGameplayAbilitySpec Spec(WeaponDA->BasicAbility.AbilityClass, 1, static_cast<int32>(EAbilityInputID::BasicAttack),this);
			CurrentWeaponAbilityHandles.Add(ASC->GiveAbility(Spec));
		}
		
		int32 SkillInputIndex = static_cast<int32>(EAbilityInputID::Skill1);
		for (const FWeaponSkillData& SkillData : WeaponInstance->GetGrantedSkills())
		{
			if (SkillData.AbilityClass)
			{
				FGameplayAbilitySpec Spec(SkillData.AbilityClass, 1, SkillInputIndex++, this);
				CurrentWeaponAbilityHandles.Add(ASC->GiveAbility(Spec));
			}
		}
	}
}

void ACAP_PlayerCharacter::ClearCurrentWeaponData()
{
	if (WeaponMesh_R)
		WeaponMesh_R->SetSkeletalMesh(nullptr);
	if (WeaponMesh_L)
		WeaponMesh_L->SetSkeletalMesh(nullptr);
	
	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC)
	{
		for (const FGameplayAbilitySpecHandle& Handle : CurrentWeaponAbilityHandles)
		{
			ASC->ClearAbility(Handle);
		}
	}
	CurrentWeaponAbilityHandles.Empty();
}
