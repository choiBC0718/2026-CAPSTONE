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
#include "Interface/CAP_InteractInterface.h"
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
	if (WeaponComponent)
		WeaponComponent->SwapWeapon();
}