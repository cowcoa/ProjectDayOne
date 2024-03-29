// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"


#include "DayOne/Component/LocomotionComponent.h"
#include "DayOne/Component/ThirdPersonCameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

FName ABaseCharacter::MoveForwardInputName(TEXT("MoveForward"));
FName ABaseCharacter::MoveRightInputName(TEXT("MoveRight"));
FName ABaseCharacter::LookupInputName(TEXT("LookUp"));
FName ABaseCharacter::TurnInputName(TEXT("Turn"));
FName ABaseCharacter::StanceInputName(TEXT("Stance"));

ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<ULocomotionComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	LookupRate = 1.25f;
	TurnRate = 1.25f;
	
	ThirdPersonCamera = CreateDefaultSubobject<UThirdPersonCameraComponent>(TEXT("ThirdPersonCamera"));
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	//PlayerInputComponent->BindAction(StanceInputName, EInputEvent::IE_Pressed, this, &ThisClass::OnStance);

	PlayerInputComponent->BindAxis(MoveForwardInputName, this, &ThisClass::OnMoveForward);
	PlayerInputComponent->BindAxis(MoveRightInputName, this, &ThisClass::OnMoveRight);
	PlayerInputComponent->BindAxis(LookupInputName, this, &ThisClass::OnLookUp);
	PlayerInputComponent->BindAxis(TurnInputName, this, &ThisClass::OnTurn);

	PlayerInputComponent->BindAction(StanceInputName, EInputEvent::IE_Pressed, this, &ThisClass::OnStance);
}

void ABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (ThirdPersonCamera)
	{
		ThirdPersonCamera->Character = this;
	}

	Locomotion = Cast<ULocomotionComponent>(GetMovementComponent());
	check(Locomotion);
	Locomotion->Character = this;
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	UE_LOG(LogTemp,Warning,TEXT("OnStartCrouch"))
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	Locomotion->SetStance(EStanceState::SS_Crouching);
}

void ABaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	UE_LOG(LogTemp,Warning,TEXT("OnEndCrouch"))
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	Locomotion->SetStance(EStanceState::SS_Standing);
}

void ABaseCharacter::OnMoveForward(float Value)
{
	ProcessPlayerMovementInput(true);
}

void ABaseCharacter::OnMoveRight(float Value)
{
	ProcessPlayerMovementInput(false);
}

void ABaseCharacter::OnLookUp(float Value)
{
	AddControllerPitchInput(Value * LookupRate);
}

void ABaseCharacter::OnTurn(float Value)
{
	AddControllerYawInput(Value * TurnRate);
}

void ABaseCharacter::OnStance()
{
	if (Locomotion->MovementAction != EMovementAction::MA_None) return;
	
	// We don't handle the 'break fall' situation of crouching in air.
	if (Locomotion->MovementState == EMovementState::MS_Grounded)
	{
		if (Locomotion->Stance == EStanceState::SS_Standing)
		{
			Locomotion->DesiredStance = EStanceState::SS_Crouching;
			Crouch();
		}
		else // Locomotion->Stance == EStanceState::SS_Crouching
		{
			Locomotion->DesiredStance = EStanceState::SS_Standing;
			UnCrouch();
		}
	}
}

/*
void ABaseCharacter::OnStance()
{
	if (MovementAction == EMovementAction::MA_None)
	{
		if (MovementState == EMovementState::MS_Grounded)
		{
			if (Stance == EStanceState::SS_Standing)
			{
				DesiredStance = EStanceState::SS_Crouching;
				Crouch();
			}
			else if (Stance == EStanceState::SS_Crouching)
			{
				DesiredStance = EStanceState::SS_Standing;
				UnCrouch();
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("DesiredStance: %d"), DesiredStance);
}
*/

void ABaseCharacter::ProcessPlayerMovementInput(float bIsForwardAxis)
{
	// Default camera relative movement behavior
	if (Locomotion->MovementState == EMovementState::MS_Grounded || Locomotion->MovementState == EMovementState::MS_InAir)
	{
		FVector ForwardVector;
		FVector RightVector;
		GetControlVector(ForwardVector, RightVector);

		float ForwardInput = GetInputAxisValue(MoveForwardInputName);
		float RightInput = GetInputAxisValue(MoveRightInputName);
		
		float FixedForwardInput;
		float FixedRightInput;
		FixDiagonalGamepadValues(ForwardInput, RightInput, FixedForwardInput, FixedRightInput);
		
		if (bIsForwardAxis)
		{
			AddMovementInput(ForwardVector, FixedForwardInput);
		}
		else
		{
			AddMovementInput(RightVector, FixedRightInput);
		}
	}
}

void ABaseCharacter::GetControlVector(FVector& Forward, FVector& Right) const
{
	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	Forward = UKismetMathLibrary::GetForwardVector(YawRotation);
	Right = UKismetMathLibrary::GetRightVector(YawRotation);
}

void ABaseCharacter::FixDiagonalGamepadValues(float InY, float InX, float& OutY, float& OutX)
{
	OutY = UKismetMathLibrary::Clamp(InY * UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Abs(InX), 0.0f, 0.6f, 1.0f, 1.2f), -1.0f, 1.0f);
	OutX = UKismetMathLibrary::Clamp(InX * UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Abs(InY), 0.0f, 0.6f, 1.0f, 1.2f), -1.0f, 1.0f);
}

EGaitState ABaseCharacter::GetGait() const
{
	return Locomotion->Gait;
}

EStanceState ABaseCharacter::GetStance() const
{
	return Locomotion->Stance;
}

void ABaseCharacter::GetEssentialValues(FVector& Velocity,
	                                    FVector& PhysicalAcceleration,
	                                    FVector& MovementInput,
	                                    bool& IsMoving,
	                                    bool& HasMovementInput,
	                                    float& Speed,
	                                    float& MovementInputAmount,
	                                    FRotator& AimingRotation,
	                                    float& AimYawRate) const
{
	Velocity = GetVelocity();
	PhysicalAcceleration = Locomotion->PhysicalAcceleration;
	MovementInput = Locomotion->GetCurrentAcceleration();
	IsMoving = Locomotion->bIsMoving;
	HasMovementInput = Locomotion->bHasMovementInput;
	Speed = Locomotion->Speed;
	MovementInputAmount = Locomotion->MovementInputAmount;
	AimingRotation = GetControlRotation();
	AimYawRate = Locomotion->AimYawRate;
}

void ABaseCharacter::GetCurrentStates(EMovementState& MovementState,
	                                  EMovementState& PrevMovementState,
	                                  EMovementAction& MovementAction,
	                                  ERotationMode& RotationMode,
	                                  EGaitState& ActualGait,
	                                  EStanceState& ActualStance) const
{
	MovementState = Locomotion->MovementState;
	PrevMovementState = Locomotion->PrevMovementState;
	MovementAction = Locomotion->MovementAction;
	RotationMode = Locomotion->RotationMode;
	ActualGait = Locomotion->Gait;
	ActualStance = Locomotion->Stance;
}
