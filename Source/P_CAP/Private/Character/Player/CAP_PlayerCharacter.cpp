// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/CAP_PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

ACAP_PlayerCharacter::ACAP_PlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm Component");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritYaw = false;

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera Component");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ACAP_PlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	APlayerController* OwningPC = GetController<APlayerController>();
	if (OwningPC)
	{
		//마우스 보이도록 and 클릭 설정

		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = OwningPC->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubsystem)
		{
			InputSubsystem->RemoveMappingContext(GameplayIMC);
			InputSubsystem->AddMappingContext(GameplayIMC,0);
		}
	}
}

void ACAP_PlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComp)
	{
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &ACAP_PlayerCharacter::MoveInputHandle);
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Completed, this, &ACAP_PlayerCharacter::MoveInputHandle);
		EnhancedInputComp->BindAction(MoveInputAction, ETriggerEvent::Canceled, this, &ACAP_PlayerCharacter::MoveInputHandle);
		
		EnhancedInputComp->BindAction(DirFixInputAction, ETriggerEvent::Started, this, &ACAP_PlayerCharacter::ToggleDirFixHandle);

		for (const TPair<EAbilityInputType, UInputAction*> InputActionPair : GameplayAbilityIAMap)
		{
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Started, this, &ACAP_PlayerCharacter::AbilityInputHandle, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Completed, this, &ACAP_PlayerCharacter::AbilityInputHandle, InputActionPair.Key);
			EnhancedInputComp->BindAction(InputActionPair.Value, ETriggerEvent::Canceled, this, &ACAP_PlayerCharacter::AbilityInputHandle, InputActionPair.Key);
		}
	}
}

void ACAP_PlayerCharacter::MoveInputHandle(const FInputActionValue& InputActionValue)
{
	FVector2D InputVal = InputActionValue.Get<FVector2D>();
	if (InputVal.IsNearlyZero())
		return;
	if (InputVal.Length() > 1.f)
		InputVal.Normalize();
	
	AddMovementInput(GetMoveForwardVector()	* InputVal.Y + GetMoveRightVector() * InputVal.X);
}

void ACAP_PlayerCharacter::ToggleDirFixHandle()
{
	GetCharacterMovement()->bOrientRotationToMovement = GetCharacterMovement()->bOrientRotationToMovement ? false : true;
}

void ACAP_PlayerCharacter::AbilityInputHandle(const FInputActionValue& InputActionValue, EAbilityInputType AbilityInput)
{
	bool bPressed = InputActionValue.Get<bool>();
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32)AbilityInput);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32)AbilityInput);
	}
}

FVector ACAP_PlayerCharacter::GetMoveForwardVector() const
{
	FVector Forward = Camera->GetForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();
	return Forward;
}

FVector ACAP_PlayerCharacter::GetMoveRightVector() const
{
	FVector Forward = Camera->GetRightVector();
	Forward.Z = 0.f;
	Forward.Normalize();
	return Forward;
}
