// Fill out your copyright notice in the Description page of Project Settings.


#include "LocomotionComponent.h"

#include "Curves/CurveVector.h"
#include "DayOne/Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ULocomotionComponent::ULocomotionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	
	DesiredGait = EGaitState::GS_Running;

	MovementAction = EMovementAction::MA_None;
	MovementState = EMovementState::MS_Grounded;
	Stance = EStanceState::SS_Standing;
	RotationMode = ERotationMode::RM_Looking;
	Gait = EGaitState::GS_Walking;
}

void ULocomotionComponent::BeginPlay()
{
	Super::BeginPlay();

	check(Character && Character->GetMesh());

	// Make sure the mesh and animbp update after this component to ensure it gets the most recent values.
	Character->GetMesh()->AddTickPrerequisiteComponent(this);
	Character->GetCameraComponent()->AddTickPrerequisiteComponent(this);
	
	// Set Reference to the Main Anim Instance.
	MainAnimInstance = Character->GetMesh()->GetAnimInstance();

	// Set the Movement Model
	SetMovementModel();

	// Update states to use the initial desired values.
	SetGait(DesiredGait);

	// Set default rotation values.
	TargetRotation = LastVelocityRotation = LastMovementInputRotation = Character->GetActorRotation();
}

// Called every frame
void ULocomotionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	{
		SetEssentialValues();
		// Check Movement Mode
		switch (MovementState)
		{
		case EMovementState::MS_Grounded:
			// Do While On Ground
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

	CacheValues();
}

void ULocomotionComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	switch (MovementMode)
	{
	case EMovementMode::MOVE_Falling:
		SetMovementState(EMovementState::MS_InAir);
		break;
	case EMovementMode::MOVE_Walking:
	case EMovementMode::MOVE_NavWalking:
		SetMovementState(EMovementState::MS_Grounded);
		break;
	default:
		checkNoEntry();
	}

	UE_LOG(LogTemp, Warning, TEXT("OnMovementModeChanged: %d"), static_cast<int32>(MovementState));
}

void ULocomotionComponent::Crouch(bool bClientSimulation)
{
	Super::Crouch(bClientSimulation);
	SetStance(EStanceState::SS_Crouching);
}

void ULocomotionComponent::UnCrouch(bool bClientSimulation)
{
	Super::UnCrouch(bClientSimulation);
	SetStance(EStanceState::SS_Standing);
}

void ULocomotionComponent::SetMovementModel()
{
	check(MovementModel);

	// Get Normal Movement Model
	const FText ResourceString = StaticEnum<EMovementModel>()->GetDisplayNameTextByIndex(static_cast<int32>(EMovementModel::MM_Normal));
	MovementData.Normal = MovementModel->FindRow<FMovementSettingsState>(*ResourceString.ToString(), "MovementModel.Normal");

	check(MovementData.Normal);
}

void ULocomotionComponent::SetGait(EGaitState NewActualGait)
{
	if (NewActualGait == Gait) return;

	static EGaitState PreviousActualGait = Gait;
	Gait = NewActualGait;
}

void ULocomotionComponent::SetMovementState(EMovementState NewMovementState)
{
	check(Character);
	
	if (NewMovementState == MovementState) return;

	PrevMovementState = MovementState;

	MovementState = NewMovementState;

	// If the character enters the air, set the In Air Rotation and uncrouch if crouched.
	// If the character is currently rolling, enable the ragdoll.
	if (MovementState == EMovementState::MS_InAir)
	{
		if (MovementAction == EMovementAction::MA_None)
		{
			InAirRotation = Character->GetActorRotation();
			if (Stance == EStanceState::SS_Crouching)
			{
				UnCrouch();
			}
		}
	}
}

void ULocomotionComponent::SetStance(EStanceState NewStance)
{
	if (NewStance == Stance) return;

	static EStanceState PreviousStance = Stance;
	Stance = NewStance;
}

void ULocomotionComponent::SetEssentialValues()
{
	check(Character);
	
	// Set the amount of Acceleration.
	PhysicalAcceleration = CalculatePhysicalAcceleration();

	// Determine if the character is moving by getting it's speed.
	// The Speed equals the length of the horizontal (x y) velocity,
	// so it does not take vertical movement into account.
	// If the character is moving, update the last velocity rotation.
	// This value is saved because it might be useful to know the last orientation of movement even after the character has stopped.
	const FVector HorizontalVelocity(Character->GetVelocity().X, Character->GetVelocity().Y, 0);
	Speed = HorizontalVelocity.Length();
	bIsMoving = Speed > 1.0f ? true : false;
	if (bIsMoving)
	{
		LastVelocityRotation = Character->GetVelocity().ToOrientationRotator();
	}

	// Determine if the character has movement input by getting its movement input amount.
	// The Movement Input Amount is equal to the current acceleration divided by the max acceleration so that it has a range of 0-1,
	// 1 being the maximum possible amount of input, and 0 being none.
	// If the character has movement input, update the Last Movement Input Rotation.
	MovementInputAmount = GetCurrentAcceleration().Length() / MaxAcceleration;
	bHasMovementInput = MovementInputAmount > 0.0f ? true : false;
	if (bHasMovementInput)
	{
		LastMovementInputRotation = GetCurrentAcceleration().ToOrientationRotator();
	}

	// Set the Aim Yaw rate by comparing the current and previous Aim Yaw value, divided by Delta Seconds.
	// This represents the speed the camera is rotating left to right. 
	AimYawRate = UKismetMathLibrary::Abs((Character->GetControlRotation().Yaw - PreviousAimYaw) / UGameplayStatics::GetWorldDeltaSeconds(this));
}

FVector ULocomotionComponent::CalculatePhysicalAcceleration() const
{
	check(Character);
	// Calculate the Acceleration by comparing the current and previous velocity.
	// The Current Acceleration returned by the movement component equals the input acceleration,
	// and does not represent the actual physical acceleration of the character.
	return (Character->GetVelocity() - PreviousVelocity) / UGameplayStatics::GetWorldDeltaSeconds(Character);
}

void ULocomotionComponent::UpdateCharacterMovement()
{
	// Set the Allowed Gait
	EGaitState AllowedGait = GetAllowedGait();

	// Determine the Actual Gait.
	// If it is different from the current Gait, Set the new Gait Event.
	EGaitState ActualGait = GetActualGait(AllowedGait);
	if (ActualGait != Gait)
	{
		SetGait(ActualGait);
	}

	// Use the allowed gait to update the movement settings.
	UpdateDynamicMovementSettings(AllowedGait);
}

EGaitState ULocomotionComponent::GetAllowedGait() const
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

bool ULocomotionComponent::CanSprint() const
{
	check(Character);
	
	if (!bHasMovementInput) return false;

	if (RotationMode == ERotationMode::RM_Aiming) return false;

	if (RotationMode == ERotationMode::RM_Looking)
	{
		// Try to find the angle of  player input direct and current player controller direct
		// If player has heavy input and input direction between with controller direction almost the same, sprint.
		FRotator MovementInputRotation = GetCurrentAcceleration().ToOrientationRotator();
		FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(MovementInputRotation, Character->GetControlRotation());
		float DeltaYaw = UKismetMathLibrary::Abs(DeltaRotation.Yaw);
		return (MovementInputAmount > 0.9f && DeltaYaw < 50.0f);
	}
	// TODO
	// if (RotationMode == ERotationMode::RM_Velocity)

	checkNoEntry();
	return false;
}

EGaitState ULocomotionComponent::GetActualGait(EGaitState AllowedGait) const
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

void ULocomotionComponent::UpdateDynamicMovementSettings(EGaitState AllowedGait)
{
	// Get the Current Movement Settings.
	CurrentMovementSettings = GetTargetMovementSettings();

	// Update the Character Max Walk Speed to the configured speeds based on the currently Allowed Gait.
	switch (AllowedGait)
	{
	case EGaitState::GS_Walking:
		MaxWalkSpeed = CurrentMovementSettings.WalkSpeed;
		MaxWalkSpeedCrouched = MaxWalkSpeed;
		break;
	case EGaitState::GS_Running:
		MaxWalkSpeed = CurrentMovementSettings.RunSpeed;
		MaxWalkSpeedCrouched = MaxWalkSpeed;
		break;
	case EGaitState::GS_Sprinting:
		MaxWalkSpeed = CurrentMovementSettings.SprintSpeed;
		MaxWalkSpeedCrouched = MaxWalkSpeed;
		break;
	}

	// Update the Acceleration, Deceleration, and Ground Friction using the Movement Curve.
	// This allows for fine control over movement behavior at each speed (May not be suitable for replication).
	FVector MovementCurveValue = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed());
	MaxAcceleration = MovementCurveValue.X;
	BrakingDecelerationWalking = MovementCurveValue.Y;
	GroundFriction = MovementCurveValue.Z;
}

FMovementSettings ULocomotionComponent::GetTargetMovementSettings() const
{
	check(MovementData.Normal)
	
	if (RotationMode == ERotationMode::RM_Looking)
	{
		if (Stance == EStanceState::SS_Standing)
		{
			return MovementData.Normal->Looking.Standing;
		}
		else if (Stance == EStanceState::SS_Crouching)
		{
			return MovementData.Normal->Looking.Crouching;
		}
	}
	else if (RotationMode == ERotationMode::RM_Aiming)
	{
		if (Stance == EStanceState::SS_Standing)
		{
			return MovementData.Normal->Aiming.Standing;
		}
		else if (Stance == EStanceState::SS_Crouching)
		{
			return MovementData.Normal->Aiming.Crouching;
		}
	}

	checkNoEntry();
	return MovementData.Normal->Looking.Standing;
}

float ULocomotionComponent::GetMappedSpeed() const
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

void ULocomotionComponent::UpdateGroudedRotation()
{
	check(Character);
	
	if (MovementAction == EMovementAction::MA_None)
	{
		if (CanUpdateMovementRotation())
		{
			// Looking Direction Rotation
			if (RotationMode == ERotationMode::RM_Looking)
			{
				if (Gait == EGaitState::GS_Walking || Gait == EGaitState::GS_Running)
				{
					float TargetYaw = Character->GetControlRotation().Yaw + GetAnimCurveValue("YawOffset");
					float GroundedRotationRate = CalculateGroundedRotationRate();
					SmoothCharacterRotation(FRotator(0.0f, TargetYaw, 0.0f), 500.0f, GroundedRotationRate);
				}
				else if (Gait == EGaitState::GS_Sprinting)
				{
					float GroundedRotationRate = CalculateGroundedRotationRate();
					SmoothCharacterRotation(FRotator(0.0f, LastVelocityRotation.Yaw, 0.0f), 500.0f, GroundedRotationRate);
				}
			}
			// Aiming Rotation
			else if (RotationMode == ERotationMode::RM_Aiming)
			{
				float GroundedRotationRate = CalculateGroundedRotationRate();
				SmoothCharacterRotation(FRotator(0.0f, Character->GetControlRotation().Yaw, 0.0f), 1000.0f, GroundedRotationRate);
			}
		}
		else
		{
			// Not Moving
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
				Character->AddActorWorldRotation(FRotator(0.0f, DeltaYaw, 0.0f));
				TargetRotation = Character->GetActorRotation();
			}
		}
	}
	else
	{
		// TODO: Rolling
		checkNoEntry();
	}
}

bool ULocomotionComponent::CanUpdateMovementRotation() const
{
	check(Character);
	return ((bIsMoving && bHasMovementInput) || Speed > 150.0f) && !Character->HasAnyRootMotion();
}

void ULocomotionComponent::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed)
{
	check(Character);
	
	TargetRotation = UKismetMathLibrary::RInterpTo_Constant(TargetRotation, Target, UGameplayStatics::GetWorldDeltaSeconds(this), TargetInterpSpeed);

	FRotator NewRotation = UKismetMathLibrary::RInterpTo(Character->GetActorRotation(), TargetRotation, UGameplayStatics::GetWorldDeltaSeconds(this), ActorInterpSpeed);
	Character->SetActorRotation(NewRotation);
}

float ULocomotionComponent::GetAnimCurveValue(FName CurveName) const
{
	if (!MainAnimInstance) return 0.0f;

	return MainAnimInstance->GetCurveValue(CurveName);
}

float ULocomotionComponent::CalculateGroundedRotationRate() const
{
	check(CurrentMovementSettings.RotationRateCurve);

	float RotationRateValue = CurrentMovementSettings.RotationRateCurve->GetFloatValue(GetMappedSpeed());
	float MappedAimYawRate = UKismetMathLibrary::MapRangeClamped(AimYawRate, 0.0f, 300.0f, 1.0f, 3.0f);

	return RotationRateValue * MappedAimYawRate;
}

void ULocomotionComponent::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed)
{
	check(Character);
	
	float DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(Character->GetControlRotation(), Character->GetActorRotation()).Yaw;
	if (!UKismetMathLibrary::InRange_FloatFloat(DeltaYaw, AimYawMin, AimYawMax, true, true))
	{
		float TargetYaw = 0.0f;
		if (DeltaYaw > 0.0f)
		{
			TargetYaw = Character->GetControlRotation().Yaw + AimYawMin;
		}
		else
		{
			TargetYaw = Character->GetControlRotation().Yaw + AimYawMax;
		}
		SmoothCharacterRotation(FRotator(0.0f, TargetYaw, 0.0f), 0.0f, InterpSpeed);
	}
}

void ULocomotionComponent::CacheValues()
{
	check(Character);
	
	PreviousVelocity = Character->GetVelocity();
	PreviousAimYaw = Character->GetControlRotation().Yaw;
}
