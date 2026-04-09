// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerCharacter.h"

#include "CAP_PlayerController.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Ability/CAP_GameplayAbility.h"
#include "Interface/CAP_InteractInterface.h"
#include "Weapon/CAP_WeaponBase.h"

ACAP_PlayerCharacter::ACAP_PlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm Comp");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritYaw = false;    
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera Comp");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	WeaponMesh_R = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMesh_R");
	WeaponMesh_R->SetupAttachment(GetMesh());
	WeaponMesh_R->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh_L = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMesh_L");
	WeaponMesh_L->SetupAttachment(GetMesh());
	WeaponMesh_L->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
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
		ASC->InitComponent();
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

	if (DefaultBasicWeapon)
	{
		EquippedWeapons[0] = DefaultBasicWeapon;
		ApplyWeaponData(DefaultBasicWeapon);
	}
	EquippedWeapons[1] = nullptr;
	CurrentWeaponIndex = 0;
}

void ACAP_PlayerCharacter::PickupWeapon(class UCAP_WeaponDataAsset* NewWeaponDA)
{
	if (!NewWeaponDA)
		return;

	int32 EmptySlotIndex = INDEX_NONE;
	for (int32 i=0; i<EquippedWeapons.Num() ; ++i)
	{	//빈 무기 슬롯 체크
		if (EquippedWeapons[i]==nullptr)
		{
			EmptySlotIndex = i;
			break;
		}
	}

	if (EmptySlotIndex != INDEX_NONE)
	{	//빈 슬롯에 무기 착용 후 바로 장착
		EquippedWeapons[EmptySlotIndex] = NewWeaponDA;
		CurrentWeaponIndex = EmptySlotIndex;

		ApplyWeaponData(NewWeaponDA);
	}
	else
	{//빈 슬롯이 없는 상태에서 무기를 주웠다면
		UCAP_WeaponDataAsset* WeaponToDrop = EquippedWeapons[CurrentWeaponIndex];
		if (WeaponToDrop)
		{
			FVector DropLocation = GetActorLocation() + (GetActorForwardVector() * 100.f);
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			

			ACAP_WeaponBase* DroppedWeapon = GetWorld()->SpawnActor<ACAP_WeaponBase>(WeaponToDrop->WeaponClass, DropLocation, FRotator::ZeroRotator, SpawnParams);
			if (DroppedWeapon)
			{
				DroppedWeapon->WeaponDA = WeaponToDrop;
				DroppedWeapon->OnConstruction(DroppedWeapon->GetTransform());
			}
		}

		EquippedWeapons[CurrentWeaponIndex] = NewWeaponDA;
		ApplyWeaponData(NewWeaponDA);
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

void ACAP_PlayerCharacter::ApplyWeaponData(class UCAP_WeaponDataAsset* WeaponDA)
{
	ClearCurrentWeaponData();
	if (!WeaponDA)
		return;

	for (const FWeaponVisualInfo& VisualInfo : WeaponDA->WeaponVisualInfos)
	{
		if (VisualInfo.EquipHand == EEquipHand::Left)
		{
			WeaponMesh_L->SetStaticMesh(VisualInfo.WeaponMesh);
			WeaponMesh_L->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, VisualInfo.EquipSocketName);
			WeaponMesh_L->SetRelativeTransform(VisualInfo.EquipTransform);
		}
		else
		{
			WeaponMesh_R->SetStaticMesh(VisualInfo.WeaponMesh);
			WeaponMesh_R->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, VisualInfo.EquipSocketName);
			WeaponMesh_R->SetRelativeTransform(VisualInfo.EquipTransform);
		}
	}

	UCAP_AbilitySystemComponent* ASC = Cast<UCAP_AbilitySystemComponent>(GetAbilitySystemComponent());
	if (ASC)
	{
		if (WeaponDA->BasicAttack.LoadSynchronous())
		{
			FGameplayAbilitySpec Spec(WeaponDA->BasicAttack.Get(), 1, static_cast<int32>(EAbilityInputID::BasicAttack));
			CurrentWeaponAbilityHandles.Add(ASC->GiveAbility(Spec));
		}
		// 액티브 스킬 배열 부여 (스킬 슬롯 규칙에 맞춰 InputID 맵핑 필요)
		int32 SkillInputIndex = static_cast<int32>(EAbilityInputID::Skill1);
		for (auto& SkillClass : WeaponDA->ActiveSkillArray)
		{
			if (SkillClass.LoadSynchronous())
			{
				FGameplayAbilitySpec Spec(SkillClass.Get(), 1, SkillInputIndex++, this);
				CurrentWeaponAbilityHandles.Add(ASC->GiveAbility(Spec));
			}
		}
	}
}

void ACAP_PlayerCharacter::ClearCurrentWeaponData()
{
	WeaponMesh_R->SetStaticMesh(nullptr);
	WeaponMesh_L->SetStaticMesh(nullptr);
	
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
