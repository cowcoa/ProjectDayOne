// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"


#include "Engine/DataTable.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

FName ABaseCharacter::MoveForwardInputName(TEXT("MoveForward"));
FName ABaseCharacter::MoveRightInputName(TEXT("MoveRight"));
FName ABaseCharacter::LookupInputName(TEXT("LookUp"));
FName ABaseCharacter::TurnInputName(TEXT("Turn"));
FName ABaseCharacter::StanceInputName(TEXT("Stance"));

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	MainAnimInstance = nullptr;
	MovementData = nullptr;

	DesiredGait = EGaitState::GS_Running;
	DesiredRotationMode = ERotationMode::RM_Looking;
	DesiredStance = EStanceState::SS_Standing;
	
	MovementState = EMovementState::MS_None;
	MovementAction = EMovementAction::MA_None;

	LookupRate = 1.25f;
	TurnRate = 1.25f;
	ThirdPersonFOV = 90.0f;

	PreviousAimYaw = 0.0f;
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	check(GetMesh());

	// Make sure the mesh and animbp update after the CharacterBP to ensure it gets the most recent values.
	GetMesh()->AddTickPrerequisiteActor(this);

	// Init properties
	// Set Reference to the Main Anim Instance.
	MainAnimInstance = GetMesh()->GetAnimInstance();

	// Get movement data from the Movement Model Data table and set the Movement Data Struct.
	// This allows you to easily switch out movement behaviors.
	// Currently we only support Normal Movement Model.
	SetMovementModel();

	// Cow added
	SetCameraModel();

	// Update states to use the initial desired values.
	OnGaitChanged(DesiredGait);
	OnRotationModeChanged(DesiredRotationMode);
	// We don't need change view mode.

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

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction(StanceInputName, EInputEvent::IE_Pressed, this, &ThisClass::OnStance);

	PlayerInputComponent->BindAxis(MoveForwardInputName, this, &ThisClass::OnMoveForward);
	PlayerInputComponent->BindAxis(MoveRightInputName, this, &ThisClass::OnMoveRight);
	PlayerInputComponent->BindAxis(LookupInputName, this, &ThisClass::OnLookUp);
	PlayerInputComponent->BindAxis(TurnInputName, this, &ThisClass::OnTurn);
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SetEssentialValues();

	
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

void ABaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	OnStanceChanged(EStanceState::SS_Crouching);
}

void ABaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	OnStanceChanged(EStanceState::SS_Standing);
}

void ABaseCharacter::GetControlVector(FVector& Forward, FVector& Right) const
{
	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);
	Forward = UKismetMathLibrary::GetForwardVector(YawRotation);
	Right = UKismetMathLibrary::GetRightVector(YawRotation);
}

void ABaseCharacter::ProcessPlayerMovementInput(float bIsForwardAxis)
{
	// Default camera relative movement behavior
	if (MovementState == EMovementState::MS_Grounded || MovementState == EMovementState::MS_InAir)
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

void ABaseCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	Super::CalcCamera(DeltaTime, OutResult);

	//OutResult.Location = FVector(OutResult.Location.X, OutResult.Location.Y, OutResult.Location.Z);

	//UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::CalcCamera called"));

	// Step 1: Get Camera Parameters from CharacterBP via the Camera Interface
	FTransform PivotTarget = GetActorTransform();
	float TPFOV = ThirdPersonFOV;

	// Step 2: Calculate Target Camera Rotation. Use the Control Rotation and interpolate for smooth camera rotation.
	//float RotationLagSpeed = CurrentCameraSettings.RotationLagSpeed;
	//FRotator CurrentRotation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraRotation();
	//FRotator TargetRotation = GetControlRotation();

	
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
	// Set the amount of Acceleration.
	Acceleration = CalculateAcceleration();

	// Determine if the character is moving by getting it's speed.
	// The Speed equals the length of the horizontal (x y) velocity,
	// so it does not take vertical movement into account.
	// If the character is moving, update the last velocity rotation.
	// This value is saved because it might be useful to know the last orientation of movement even after the character has stopped.
	const FVector HorizontalVelocity(GetVelocity().X, GetVelocity().Y, 0);
	Speed = HorizontalVelocity.Length();
	bIsMoving = Speed > 1.0f ? true : false;
	if (bIsMoving)
	{
		LastVelocityRotation = GetVelocity().ToOrientationRotator();
		
	}

	// Determine if the character has movement input by getting its movement input amount.
	// The Movement Input Amount is equal to the current acceleration divided by the max acceleration so that it has a range of 0-1,
	// 1 being the maximum possible amount of input, and 0 being none.
	// If the character has movement input, update the Last Movement Input Rotation.
	MovementInputAmount = GetCharacterMovement()->GetCurrentAcceleration().Length() / GetCharacterMovement()->MaxAcceleration;
	bHasMovementInput = MovementInputAmount > 0.0f ? true : false;
	if (bHasMovementInput)
	{
		LastMovementInputRotation = GetCharacterMovement()->GetCurrentAcceleration().ToOrientationRotator();
	}

	// Set the Aim Yaw rate by comparing the current and previous Aim Yaw value, divided by Delta Seconds.
	// This represents the speed the camera is rotating left to right. 
	AimYawRate = UKismetMathLibrary::Abs((GetControlRotation().Yaw - PreviousAimYaw) / UGameplayStatics::GetWorldDeltaSeconds(this));

	// Check Movement Mode
	switch (MovementState)
	{
	case EMovementState::MS_Grounded:
		UpdateCharacterMovement();
		UpdateGroudedRotation();
		break;
	case EMovementState::MS_InAir:
		// TODO
		checkNoEntry();
		break;
	default:
		checkNoEntry();
	}
}

void ABaseCharacter::CacheValues()
{
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = GetControlRotation().Yaw;
}

void ABaseCharacter::SetMovementModel()
{
	check(MovementModel);

	// Get Normal Movement Model
	const FText ResourceString = StaticEnum<EMovementModel>()->GetDisplayNameTextByIndex(static_cast<int32>(EMovementModel::MM_Normal));
	MovementData = MovementModel->FindRow<FMovementSettingsState>(*ResourceString.ToString(), "Movement Settings State Context");
}

void ABaseCharacter::OnStanceChanged(EStanceState NewStance)
{
	if (NewStance == Stance) return;

	static EStanceState PreviousStance = Stance;
	Stance = NewStance;
}

EGaitState ABaseCharacter::GetAllowedGait() const
{
	EGaitState AllowedGait = EGaitState::GS_MAX;
	
	if (Stance == EStanceState::SS_Standing)
	{
		if (RotationMode == ERotationMode::RM_Looking || RotationMode == ERotationMode::RM_Velocity)
		{
			AllowedGait = DesiredGait;
			if (DesiredGait == EGaitState::GS_Sprinting)
			{
				AllowedGait = CanSprint() ? EGaitState::GS_Sprinting : EGaitState::GS_Running;
			}
		}
		else if (RotationMode == ERotationMode::RM_Aiming)
		{
			AllowedGait = EGaitState::GS_Running;
			if (DesiredGait == EGaitState::GS_Walking)
			{
				AllowedGait = EGaitState::GS_Walking;
			}
		}
	}
	else if (Stance == EStanceState::SS_Crouching)
	{
		AllowedGait = EGaitState::GS_Running;
		if (DesiredGait == EGaitState::GS_Walking)
		{
			AllowedGait = EGaitState::GS_Walking;
		}
	}

	return AllowedGait;
}

EGaitState ABaseCharacter::GetActualGait(EGaitState AllowedGait) const
{
	float LocalWalkSpeed = CurrentMovementSettings.WalkSpeed;
	float LocalRunSpeed = CurrentMovementSettings.RunSpeed;
	float LocalSprintSpeed = CurrentMovementSettings.SprintSpeed;

	EGaitState ActualGait = EGaitState::GS_MAX;

	if (Speed >= (LocalRunSpeed + 10.0f))
	{
		ActualGait = EGaitState::GS_Running;
		if (AllowedGait == EGaitState::GS_Sprinting)
		{
			ActualGait = EGaitState::GS_Sprinting;
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

bool ABaseCharacter::CanSprint() const
{
	if (!bHasMovementInput) return false;

	if (RotationMode == ERotationMode::RM_Aiming) return false;

	if (RotationMode == ERotationMode::RM_Looking)
	{
		FRotator MovementInputRotation = GetCharacterMovement()->GetCurrentAcceleration().ToOrientationRotator();
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(MovementInputRotation, GetControlRotation());
		float DeltaYaw = UKismetMathLibrary::Abs(DeltaRotation.Yaw);
		return (MovementInputAmount > 0.9f && DeltaYaw < 50.0f);
	}
	// TODO
	// if (RotationMode == ERotationMode::RM_Velocity)

	checkNoEntry();
	return false;
}

void ABaseCharacter::UpdateCharacterMovement()
{
	// Set the Allowed Gait
	EGaitState AllowedGait = GetAllowedGait();

	// Determine the Actual Gait.
	// If it is different from the current Gait, Set the new Gait Event.
	EGaitState ActualGait = GetActualGait(AllowedGait);
	if (ActualGait != Gait)
	{
		OnGaitChanged(ActualGait);
	}

	// Use the allowed gait to update the movement settings.
	UpdateDynamicMovementSettings(AllowedGait);

	// Cow Add
	CurrentCameraSettings = GetTargetCameraSettings();
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
			else if (RotationMode == ERotationMode::RM_Aiming)
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

bool ABaseCharacter::CanUpdateMovementRotation() const
{
	return ((bIsMoving && bHasMovementInput) || Speed > 150.0f) && !HasAnyRootMotion();
}

FMovementSettings ABaseCharacter::GetTargetMovementSettings() const    
{
	check(MovementData)
	
	if (RotationMode == ERotationMode::RM_Looking)
	{
		if (Stance == EStanceState::SS_Standing)
		{
			return MovementData->Looking.Standing;
		}
		else if (Stance == EStanceState::SS_Crouching)
		{
			return MovementData->Looking.Crouching;
		}
	}
	else if (RotationMode == ERotationMode::RM_Aiming)
	{
		if (Stance == EStanceState::SS_Standing)
		{
			return MovementData->Aiming.Standing;
		}
		else if (Stance == EStanceState::SS_Crouching)
		{
			return MovementData->Aiming.Crouching;
		}
	}

	checkNoEntry();
	return MovementData->Looking.Standing;
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

float ABaseCharacter::GetAnimCurveValue(FName CurveName) const
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
	if (!UKismetMathLibrary::InRange_FloatFloat(DeltaYaw, AimYawMin, AimYawMax, true, true))
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

void ABaseCharacter::OnGaitChanged(EGaitState NewActualGait)
{
	if (NewActualGait == Gait) return;

	static EGaitState PreviousActualGait = Gait;
	Gait = NewActualGait;
}

void ABaseCharacter::OnRotationModeChanged(ERotationMode NewRotationMode)
{
	if (NewRotationMode == RotationMode) return;

	static ERotationMode PreviousRotationMode = RotationMode;
	RotationMode = NewRotationMode;

	// If the new rotation mode is Velocity Direction and the character is in First Person, set the viewmode to Third Person.
	// Forget it.
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

FCameraSettings ABaseCharacter::GetTargetCameraSettings()
{
	if (Stance == EStanceState::SS_Standing)
	{
		switch (Gait)
		{
		case EGaitState::GS_Walking:
			return CameraData.Standing->Walking;
		case EGaitState::GS_Running:
			return CameraData.Standing->Running;
		case EGaitState::GS_Sprinting:
			return CameraData.Standing->Sprinting;
		default:
			checkNoEntry();
		}
	}
	else if (Stance == EStanceState::SS_Crouching)
	{
		switch (Gait)
		{
		case EGaitState::GS_Walking:
			return CameraData.Crouching->Walking;
		case EGaitState::GS_Running:
			return CameraData.Crouching->Running;
		case EGaitState::GS_Sprinting:
			return CameraData.Crouching->Sprinting;
		default:
			checkNoEntry();
		}
	}

	checkNoEntry();
	return CameraData.Crouching->Walking;
}
