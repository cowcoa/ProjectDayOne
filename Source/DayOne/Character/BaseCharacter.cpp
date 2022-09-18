// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"


#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/ITargetDevice.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MainAnimInstance = nullptr;
	MovementData = nullptr;

	DesiredGait = EGaitState::GS_Running;
	DesiredRotationMode = ERotationMode::RM_Looking;
	DesiredStance = EStanceState::SS_Standing;
	LookupRate = 1.25f;
	TurnRate = 1.25f;

	MovementState = EMovementState::MS_None;
	MovementAction = EMovementAction::MA_None;
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::OnMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::OnMoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::OnTurn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::OnLookUp);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	check(GetMesh());
	
	// Make sure the mesh and animbp update after the CharacterBP to ensure it gets the most recent values.
	GetMesh()->AddTickPrerequisiteActor(this);

	// Set Reference to the Main Anim Instance.
	MainAnimInstance = GetMesh()->GetAnimInstance();

	// Get movement data from the Movement Model Data table and set the Movement Data Struct.
	// This allows you to easily switch out movement behaviors.
	SetMovementModel();

	// Cow added
	SetCameraModel();

	// Update states to use the initial desired values.
	OnGaitChanged(DesiredGait);
	OnRotationModeChanged(DesiredRotationMode);

	switch (DesiredStance)
	{
	case EStanceState::SS_Standing:
		UnCrouch();
		break;
	case EStanceState::SS_Crouching:
		Crouch();
		break;
	default:
		checkNoEntry();
	}

	// Set default rotation values.
	TargetRotation = LastVelocityRotation = LastMovementInputRotation = GetActorRotation();
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetEssentialValues();
	//
	switch (MovementState)
	{
	case EMovementState::MS_Grounded:
		UpdateCharacterMovement();
		UpdateGroudedRotation();
	}
	
	CacheValues(); 
	// TODO 
}

void ABaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	switch (GetCharacterMovement()->MovementMode)
	{
	case EMovementMode::MOVE_Falling:
		OnMovementStateChanged(EMovementState::MS_InAir);
		break;
	case EMovementMode::MOVE_Walking:
	case EMovementMode::MOVE_NavWalking:
		OnMovementStateChanged(EMovementState::MS_Grounded);
		break;
	default:
		checkNoEntry();
	}

	UE_LOG(LogTemp, Warning, TEXT("OnMovementModeChanged: %d"), static_cast<int32>(MovementState));
}

void ABaseCharacter::GetControlVector(FVector& Forward, FVector& Right)
{
	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	Forward = UKismetMathLibrary::GetForwardVector(YawRotation);
	Right = UKismetMathLibrary::GetRightVector(YawRotation);
}

void ABaseCharacter::ProcessPlayerMovementInput(float bIsForwardAxis)
{
	if (MovementState == EMovementState::MS_Grounded || MovementState == EMovementState::MS_InAir)
	{
		FVector ForwardVector;
		FVector RightVector;
		GetControlVector(ForwardVector, RightVector);

		float ForwardInput = GetInputAxisValue(FName("MoveForward"));
		float RightInput = GetInputAxisValue(FName("MoveRight"));
		
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

void ABaseCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	Super::CalcCamera(DeltaTime, OutResult);

	//OutResult.Location = FVector(OutResult.Location.X, OutResult.Location.Y, OutResult.Location.Z);

	//UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::CalcCamera called"));

	
}

// Called every frame


// Called to bind functionality to input


void ABaseCharacter::FixDiagonalGamepadValues(float InY, float InX, float& OutY, float& OutX)
{
	OutY = UKismetMathLibrary::Clamp(InY * UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Abs(InX), 0.0f, 0.6f, 1.0f, 1.2f), -1.0f, 1.0f);
	OutX = UKismetMathLibrary::Clamp(InX * UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Abs(InY), 0.0f, 0.6f, 1.0f, 1.2f), -1.0f, 1.0f);
}

void ABaseCharacter::SetEssentialValues()
{
	Acceleration = CalculateAcceleration();

	//
	const FVector HorizontalVelocity(GetVelocity().X, GetVelocity().Y, 0);
	Speed = HorizontalVelocity.Length();
	bIsMoving = Speed > 1.0f ? true : false;
	if (bIsMoving)
	{
		LastVelocityRotation = UKismetMathLibrary::Conv_VectorToRotator(GetVelocity());
	}

	//
	MovementInputAmount = GetCharacterMovement()->GetCurrentAcceleration().Length() / GetCharacterMovement()->MaxAcceleration;
	bHasMovementInput = MovementInputAmount > 0.0f ? true : false;
	if (bHasMovementInput)
	{
		LastMovementInputRotation = UKismetMathLibrary::Conv_VectorToRotator(GetCharacterMovement()->GetCurrentAcceleration());
	}

	//
	AimYawRate = UKismetMathLibrary::Abs((GetControlRotation().Yaw - PreviousAimYaw) / UGameplayStatics::GetWorldDeltaSeconds(this));
}

void ABaseCharacter::CacheValues()
{
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = GetControlRotation().Yaw;
}

void ABaseCharacter::SetMovementModel()
{
	check(MovementModel);
	
	const FText ResourceString = StaticEnum<EMovementModel>()->GetDisplayNameTextByIndex(static_cast<int32>(EMovementModel::MM_Normal));
	MovementData = MovementModel->FindRow<FMovementSettingsState>(*ResourceString.ToString(), "Movement Settings State Context");
}

EGaitState ABaseCharacter::GetAllowedGait()
{
	EGaitState AllowedGait = EGaitState::GS_MAX;
	
	if (Stance == EStanceState::SS_Standing)
	{
		if (RotationMode == ERotationMode::RM_Looking)
		{
			switch (DesiredGait)
			{
			case EGaitState::GS_Walking:
				AllowedGait = EGaitState::GS_Walking;
				break;
			case EGaitState::GS_Running:
				AllowedGait = EGaitState::GS_Running;
				break;
			case EGaitState::GS_Sprinting:
				if (CanSprint())
				{
					AllowedGait = EGaitState::GS_Sprinting;
				}
				else
				{
					AllowedGait = EGaitState::GS_Running;
				}
				break;
			default:
				checkNoEntry();
			}
		}
		else // if (RotationMode == ERotationMode::RM_Aiming)
		{
			if (DesiredGait == EGaitState::GS_Walking)
			{
				AllowedGait = EGaitState::GS_Walking;
			}
			else if (DesiredGait == EGaitState::GS_Running || DesiredGait == EGaitState::GS_Sprinting)
			{
				AllowedGait = EGaitState::GS_Running;
			}
		}
	}
	else // if (Stance == EStanceState::SS_Crouching)
	{
		if (DesiredGait == EGaitState::GS_Walking)
		{
			AllowedGait = EGaitState::GS_Walking;
		}
		else if (DesiredGait == EGaitState::GS_Running || DesiredGait == EGaitState::GS_Sprinting)
		{
			AllowedGait = EGaitState::GS_Running;
		}
	}

	return AllowedGait;
}

EGaitState ABaseCharacter::GetActualGait(EGaitState AllowedGait)
{
	float LocalWalkSpeed = CurrentMovementSettings.WalkSpeed;
	float LocalRunSpeed = CurrentMovementSettings.RunSpeed;
	float LocalSprintSpeed = CurrentMovementSettings.SprintSpeed;

	EGaitState ActualGait = EGaitState::GS_MAX;

	if (Speed >= (LocalRunSpeed + 10.0f))
	{
		switch (AllowedGait)
		{
		case EGaitState::GS_Walking:
		case EGaitState::GS_Running:
			ActualGait = EGaitState::GS_Running;
			break;
		case EGaitState::GS_Sprinting:
			ActualGait = EGaitState::GS_Sprinting;
			break;
		default:
			checkNoEntry();
		}
	}
	else
	{
		if (Speed >= (LocalWalkSpeed + 10.0f))
		{
			ActualGait = EGaitState::GS_Running;
		}
		else
		{
			ActualGait = EGaitState::GS_Walking;
		}
	}

	return ActualGait;
}

bool ABaseCharacter::CanSprint()
{
	if (!bHasMovementInput) return false;

	if (RotationMode == ERotationMode::RM_Looking)
	{
		FRotator MovementInputRotation = UKismetMathLibrary::Conv_VectorToRotator(GetCharacterMovement()->GetCurrentAcceleration());
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(MovementInputRotation, GetControlRotation());
		return (MovementInputAmount > 0.9f && DeltaRotation.Yaw < 50.0f);
	}
	else // if (RotationMode == ERotationMode::RM_Aiming)
	{
		return false;
	}
}

void ABaseCharacter::UpdateCharacterMovement()
{
	// Set the Allowed Gait
	EGaitState AllowedGait = GetAllowedGait();

	// Determine the Actual Gait. If it is different from the current Gait, Set the new Gait Event.
	EGaitState ActualGait = GetActualGait(AllowedGait);
	if (ActualGait != AllowedGait)
	{
		OnGaitChanged(ActualGait);
	}

	// Use the allowed gait to update the movement settings.
	UpdateDynamicMovementSettings(AllowedGait);
}

void ABaseCharacter::UpdateGroudedRotation()
{
	if (MovementAction == EMovementAction::MA_None)
	{
		if (CanUpdateMovementRotation())
		{
			if (RotationMode == ERotationMode::RM_Looking)
			{
				if (Gait == EGaitState::GS_Walking || Gait == EGaitState::GS_Running)
				{
					float TargetYaw = GetControlRotation().Yaw + GetAnimCurveValue("YawOffset");
					float GroundedRotationRate = CalculateGroundedRotationRate();
					SmoothCharacterRotation(FRotator(0.0f, TargetYaw, 0.0f), 500.0f, GroundedRotationRate);
				}
				else if (Gait == EGaitState::GS_Sprinting)
				{
					float GroundedRotationRate = CalculateGroundedRotationRate();
					SmoothCharacterRotation(FRotator(0.0f, LastVelocityRotation.Yaw, 0.0f), 500.0f, GroundedRotationRate);
				}
			}
			else // if (RotationMode == ERotationMode::RM_Aiming)
			{
				float GroundedRotationRate = CalculateGroundedRotationRate();
				SmoothCharacterRotation(FRotator(0.0f, GetControlRotation().Yaw, 0.0f), 1000.0f, GroundedRotationRate);
			}
		}
		else // Not Moving
		{
			// We always in 3rd person view mode.
			if (RotationMode == ERotationMode::RM_Aiming)
			{
				LimitRotation(-100.0f, 100.0f, 20.0f);
			}
			
			// Apply the RotationAmount curve from Turn In Place Animations.
			// The Rotation Amount curve defines how much rotation should be applied each frame,
			// and is calculated for animations that are animated at 30fps.
			float RotationAmount = GetAnimCurveValue("RotationAmount");
			if (FMath::Abs(RotationAmount) > 0.001f)
			{
				float DeltaYaw = RotationAmount * (UGameplayStatics::GetWorldDeltaSeconds(this) / (1.0f / 30.0f));
				AddActorWorldRotation(FRotator(0.0f, DeltaYaw, 0.0f));
				TargetRotation = GetActorRotation();
			}
		}
	}
	else
	{
		// TODO: Rolling
		checkNoEntry();
	}
}

bool ABaseCharacter::CanUpdateMovementRotation()
{
	if (((bIsMoving && bHasMovementInput) || Speed > 150.0f) && !HasAnyRootMotion())
	{
		return true;
	}
	return false;
}

FMovementSettings ABaseCharacter::GetTargetMovementSettings()
{
	check(MovementData)
	
	if (RotationMode == ERotationMode::RM_Looking)
	{
		if (Stance == EStanceState::SS_Standing)
		{
			return MovementData->Looking.Standing;
		}
		else // if (Stance == EStanceState::SS_Crouching)
		{
			return MovementData->Looking.Crouching;
		}
	}
	else // if (RotationMode == ERotationMode::RM_Aiming)
	{
		if (Stance == EStanceState::SS_Standing)
		{
			return MovementData->Aiming.Standing;
		}
		else // if (Stance == EStanceState::SS_Crouching)
		{
			return MovementData->Aiming.Crouching;
		}
	}
}

void ABaseCharacter::UpdateDynamicMovementSettings(EGaitState AllowedGait)
{
	// Get the Current Movement Settings.
	CurrentMovementSettings = GetTargetMovementSettings();

	// Update the Character Max Walk Speed to the configured speeds based on the currently Allowed Gait.
	switch (AllowedGait)
	{
	case EGaitState::GS_Walking:
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.WalkSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeed;
		break;
	case EGaitState::GS_Running:
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.RunSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeed;
		break;
	case EGaitState::GS_Sprinting:
		GetCharacterMovement()->MaxWalkSpeed = CurrentMovementSettings.SprintSpeed;
		GetCharacterMovement()->MaxWalkSpeedCrouched = GetCharacterMovement()->MaxWalkSpeed;
		break;
	}

	// Update the Acceleration, Deceleration, and Ground Friction using the Movement Curve.
	// This allows for fine control over movement behavior at each speed (May not be suitable for replication).
	
}

float ABaseCharacter::GetAnimCurveValue(FName CurveName)
{
	if (!MainAnimInstance) return 0.0f;

	return MainAnimInstance->GetCurveValue(CurveName);
}

void ABaseCharacter::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed)
{
	TargetRotation = UKismetMathLibrary::RInterpTo_Constant(TargetRotation, Target, UGameplayStatics::GetWorldDeltaSeconds(this), TargetInterpSpeed);

	FRotator NewRotation = UKismetMathLibrary::RInterpTo(GetActorRotation(), TargetRotation, UGameplayStatics::GetWorldDeltaSeconds(this), ActorInterpSpeed);
	SetActorRotation(NewRotation);
}

float ABaseCharacter::CalculateGroundedRotationRate()
{
	check(CurrentMovementSettings.RotationRateCurve);

	float RotationRate = CurrentMovementSettings.RotationRateCurve->GetFloatValue(GetMappedSpeed());
	float MappedAimYawRate = UKismetMathLibrary::MapRangeClamped(AimYawRate, 0.0f, 300.0f, 1.0f, 3.0f);

	return RotationRate * MappedAimYawRate;
}

float ABaseCharacter::GetMappedSpeed()
{
	float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	float LocRunSpeed = CurrentMovementSettings.RunSpeed;
	float LocSprintSpeed = CurrentMovementSettings.SprintSpeed;

	float MappedWalkSpeed = UKismetMathLibrary::MapRangeClamped(Speed, 0.0f, LocWalkSpeed, 0.0f, 1.0f);
	float MappedRunSpeed = UKismetMathLibrary::MapRangeClamped(Speed, LocWalkSpeed, LocRunSpeed, 1.0f, 2.0f);
	float MappedSprintSpeed = UKismetMathLibrary::MapRangeClamped(Speed, LocRunSpeed, LocSprintSpeed, 2.0f, 3.0f);

	if (Speed > LocRunSpeed)
	{
		return MappedSprintSpeed;
	}
	else if (Speed > LocWalkSpeed)
	{
		return MappedRunSpeed;
	}
	else
	{
		return MappedWalkSpeed;
	}
}

void ABaseCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed)
{
	float DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(GetControlRotation(), GetActorRotation()).Yaw;
	if (!UKismetMathLibrary::InRange_FloatFloat(DeltaYaw, AimYawMin, AimYawMax))
	{
		float TargetYaw = 0.0f;
		if (DeltaYaw > 0.0f)
		{
			TargetYaw = GetControlRotation().Yaw + AimYawMin;
		}
		else
		{
			TargetYaw = GetControlRotation().Yaw + AimYawMax;
		}
		SmoothCharacterRotation(FRotator(0.0f, TargetYaw, 0.0f), 0.0f, InterpSpeed);
	}
}

void ABaseCharacter::OnGaitChanged(EGaitState NewState)
{
	if (NewState == Gait) return;
	
	Gait = NewState;
}

void ABaseCharacter::OnRotationModeChanged(ERotationMode NewMode)
{
	RotationMode = NewMode;
}

FVector ABaseCharacter::CalculateAcceleration() const
{
	return (GetVelocity() - PreviousVelocity) / UGameplayStatics::GetWorldDeltaSeconds(this);
}

void ABaseCharacter::OnMovementStateChanged(EMovementState NewState)
{
	if (NewState == MovementState) return;

	MovementState = NewState;

	if (MovementState == EMovementState::MS_InAir)
	{
		if (MovementAction == EMovementAction::MA_None)
		{
			InAirRotation = GetActorRotation();
			if (Stance == EStanceState::SS_Crouching)
			{
				UnCrouch();
			}
		}
	}
}

void ABaseCharacter::SetCameraModel()
{
	check(CameraModel);
	
	const FText StandingString = StaticEnum<EStanceState>()->GetDisplayNameTextByIndex(static_cast<int32>(EStanceState::SS_Standing));
	CameraData.Standing = CameraModel->FindRow<FCameraSettingsGait>(*StandingString.ToString(), "Camera In Standing Context");

	const FText CrouchingString = StaticEnum<EStanceState>()->GetDisplayNameTextByIndex(static_cast<int32>(EStanceState::SS_Crouching));
	CameraData.Crouching = CameraModel->FindRow<FCameraSettingsGait>(*CrouchingString.ToString(), "Camera In Crouching Context");

	check(CameraData.Standing);
	check(CameraData.Crouching);
}

void ABaseCharacter::GetTargetCameraSettings()
{
	
}
