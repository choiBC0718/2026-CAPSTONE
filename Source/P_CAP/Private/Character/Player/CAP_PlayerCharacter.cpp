// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerCharacter.h"

#include "CAP_PlayerController.h"
#include "AI/PlayerTrackerComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "GAS/Setting/CAP_AbilitySystemStatics.h"
#include "Items/Item/CAP_InventoryComponent.h"
#include "Items/Weapon/CAP_WeaponComponent.h"

ACAP_PlayerCharacter::ACAP_PlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm Comp");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritYaw = false;    
	SpringArm->bDoCollisionTest = false;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera Comp");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	WeaponComponent = CreateDefaultSubobject<UCAP_WeaponComponent>("Weapon Component");
	InventoryComponent = CreateDefaultSubobject<UCAP_InventoryComponent>("Inventory Component");
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	PlayerTracker = CreateDefaultSubobject<UPlayerTrackerComponent>(TEXT("Player Tracker"));
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
		
		for (const TPair<EAbilityInputID, UInputAction*>& InputActionPair : AbilityInputActions)
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
}


void ACAP_PlayerCharacter::UpdateInteractUI(bool bVisible)
{
	ACAP_PlayerController* PC = Cast<ACAP_PlayerController>(GetController());
	if (!PC)
		return;
	
	PC->UpdateInteractUI(bVisible, GetInteractKeyName());
}

void ACAP_PlayerCharacter::UpdateInteractProgress(float Progress)
{
	ACAP_PlayerController* PC = Cast<ACAP_PlayerController>(GetController());
	if (PC)
	{
		PC->UpdateInteractProgressUI(Progress);
	}
}

FString ACAP_PlayerCharacter::GetInteractKeyName() const
{
	FString CurrentKeyName = "F";
	if (ACAP_PlayerController* PC = Cast<ACAP_PlayerController>(GetController()))
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
	return CurrentKeyName;
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
	if (GetAbilitySystemComponent()->HasMatchingGameplayTag(UCAP_AbilitySystemStatics::GetMovementBlockStateTag()))
		return;
	
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
	if (InventoryComponent)
		InventoryComponent->ProcessInteractInput(Instance.GetTriggerEvent(), Instance.GetElapsedTime());
}

void ACAP_PlayerCharacter::SwapWeapon()
{
	if (WeaponComponent)
		WeaponComponent->SwapWeapon();
}

void ACAP_PlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	APlayerController* PC = GetController<APlayerController>();
	if (PC)
	{
		if (bEnabled)
		{
			EnableInput(PC);
		}
		else
		{
			DisableInput(PC);
		}
	}
}

void ACAP_PlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
}

void ACAP_PlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);
}
