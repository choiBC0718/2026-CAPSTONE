// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Data/CAP_WeaponDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Interface/CAP_InteractInterface.h"

ACAP_PlayerCharacter::ACAP_PlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm Comp");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritYaw = false;    
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera Comp");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

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
		EnhancedInputComp->BindAction(InteractIA, ETriggerEvent::Started, this, &ACAP_PlayerCharacter::InteractInputHandle);
		
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
	}
	EquippedWeapons[1] = nullptr;
	CurrentWeaponIndex = 0;

	// EquipedWeapons[0]의 스킬 부여
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
		//새로 장착한 무기 스킬 부여 & 메쉬 변경
	}
	else
	{//빈 슬롯이 없는 상태에서 무기를 주웠다면
		UCAP_WeaponDataAsset* WeaponToDrop = EquippedWeapons[CurrentWeaponIndex];
		if (WeaponToDrop)
		{
			//무기 액터 월드에 스폰 (떨어뜨리는 로직)
			//해당 무기 스킬 제거
		}
		EquippedWeapons[CurrentWeaponIndex] = NewWeaponDA;

		//주운 새로운 무기 스킬 부여
	}
}

void ACAP_PlayerCharacter::SwapWeapon()
{
	int32 NextIndex = (CurrentWeaponIndex ==0) ? 1:0;
	if (EquippedWeapons[NextIndex] == nullptr)
		return;

	CurrentWeaponIndex = NextIndex;
	// 제거된 무기의 스킬 제거
	// 변경된 무기의 스킬 부여
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

void ACAP_PlayerCharacter::InteractInputHandle(const FInputActionValue& InputActionValue)
{
	if (InteractableActor)
	{
		ICAP_InteractInterface* InteractableObj = Cast<ICAP_InteractInterface>(InteractableActor);
		if (InteractableObj)
		{
			InteractableObj->Interact(this);
		}
	}
}
