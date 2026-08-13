// Copyright DevS


#include "Player/HeroPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AHeroPlayerController::AHeroPlayerController()
{
	bReplicates = true;
}

void AHeroPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem ->AddMappingContext(HeroIMC, 0);
	}
}

void AHeroPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* HeroInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);
	
	HeroInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Move);
	HeroInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
	HeroInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Input_Jump);
	HeroInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::Input_Crouch);
	
}

void AHeroPlayerController::Input_Crouch()
{
	if (!IsValid(GetCharacter())) return;
	
	if (UCharacterMovementComponent* CMC = GetCharacter()->GetCharacterMovement(); IsValid(CMC))
	{
		CMC->bWantsToCrouch = !CMC->bWantsToCrouch;
	}
}

void AHeroPlayerController::Input_Jump()
{
	if (!IsValid(GetCharacter())) return;
	
	UCharacterMovementComponent* CMC = GetCharacter()->GetCharacterMovement();
	if (CMC->bWantsToCrouch)
	{
		CMC->bWantsToCrouch = false;    // 앉은 상태에서 점프 → 기립만
	}
	else
	{
		GetCharacter()->Jump();          // 내장 = CMC 경유 = 복제됨
	}
}

void AHeroPlayerController::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);  // (Pitch, Yaw, Roll) — 피치·롤 = 0 
	
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
	
}

void AHeroPlayerController::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	AddYawInput(InputAxisVector.X);
	AddPitchInput(InputAxisVector.Y);
}


