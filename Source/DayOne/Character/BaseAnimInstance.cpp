// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAnimInstance.h"

#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

FBaseAnimInstanceProxy::FBaseAnimInstanceProxy()
	: Super()
{
}

FBaseAnimInstanceProxy::FBaseAnimInstanceProxy(UAnimInstance* Instance)
	: Super(Instance)
{
}

void FBaseAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);
	
	// UE_LOG(LogTemp, Warning, TEXT("FBaseAnimInstanceProxy::InitializeObjects"));

	Character = Cast<ABaseCharacter>(InAnimInstance->TryGetPawnOwner());
	if (Character)
	{
		MovementComponent = Character->GetLocomotionComponent();
	}
}

void FBaseAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);

	// UE_LOG(LogTemp, Warning, TEXT("FBaseAnimInstanceProxy::PreUpdate"));
	
	UpdateCharacterInfo();
}

void FBaseAnimInstanceProxy::UpdateCharacterInfo()
{
	if (Character && MovementComponent)
	{
		ActorRotation = Character->GetActorRotation();
		MaxMovementInput = MovementComponent->GetMaxAcceleration();
		MaxBrakingDeceleration = MovementComponent->GetMaxBrakingDeceleration();
		Character->GetEssentialValues(Velocity, PhysicalAcceleration, MovementInput,
								  bIsMoving,bHasMovementInput, Speed,
								  MovementInputAmount, AimingRotation,AimYawRate);

		Character->GetCurrentStates(MovementState, PrevMovementState, MovementAction,
									RotationMode, Gait, Stance);

		bIsMovingOnGround = MovementComponent->IsMovingOnGround();
		LastUpdateRotation = MovementComponent->GetLastUpdateRotation();

		CapsuleLocation = Character->GetCapsuleComponent()->GetComponentLocation();
		Character->GetCapsuleComponent()->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
	}
}

// -----------------------------------------------------------------------------

void UBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	SmoothedAimingRotationInterpSpeed = 10.0f;
	VelocityBlendInterpSpeed = 12.0f;
	GroundedLeanInterpSpeed = 4.0f;
	AnimatedWalkSpeed = 150.0f;
	AnimatedRunSpeed = 350.0f;
	AnimatedSprintSpeed = 600.0f;
	AnimatedCrouchSpeed = 150.0f;
	RotateMinThreshold = -50.0f;
	RotateMaxThreshold = 50.0f;
	AimYawRateMinRange = 90.0f;
	AimYawRateMaxRange = 270.0f;
	MinPlayRate = 1.15f;
	MaxPlayRate = 3.0f;
	RotateRate = 1.0f;
	TurnCheckMinAngle = 45.0f;
	AimYawRateLimit = 50.0f;
	MinAngleDelay = 0.75f;
	MaxAngleDelay = 0.0f;
	Turn180Threshold = 130.0f;
	StandingPlayRate = 1.0f;
	// Layer Blending
	EnableAimOffset = 1.0f;
	BasePoseN = 1.0f;
	EnableHandIKL = 1.0f;
	EnableHandIKR = 1.0f;
	// In Air
	LandPrediction = 1.0f;
	// Config
	IKTraceDistanceAboveFoot = 50.0f;
	IKTraceDistanceBelowFoot = 45.0f;
	FootHeight = 13.5f;
}

void UBaseAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	// Only update if character is valid
	if (DeltaSeconds == 0.0f) return;
	if (Proxy->Character == nullptr) return;

	UpdateCharacterInfo(DeltaSeconds);
	UpdateAimingValues(DeltaSeconds);
	UpdateLayerValues(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);

	// Check Movement Mode
	if (Proxy->MovementState == EMovementState::MS_Grounded)
	{
		// Check If Moving Or Not
		bShouldMove = ShouldMoveCheck();
		static bool bChangedToTrueLogicGate = true;
		static bool bChangedToFalseLogicGate = true;
		if (bShouldMove)
		{
			// If ChangedToTrue logic-gate is open
			if (bChangedToTrueLogicGate)
			{
				// Reset ChangedToFalse logic-gate
				bChangedToFalseLogicGate = true;
				// Close ChangedToTrue logic-gate
				bChangedToTrueLogicGate = false;
				
				// And TODO ChangedToTrue logic
				// Do When Starting To Move
				ElapsedDelayTime = 0.0f;
				bRotateL = bRotateR = false;
			}
			// Anyway, TODO WhileTrue logic
			// Do While Moving
			UpdateMovementValues(DeltaSeconds);
			UpdateRotationValues();
		}
		else
		{
			// If ChangedToFalse logic-gate is open
			if (bChangedToFalseLogicGate)
			{
				// Reset ChangedToTrue logic-gate
				bChangedToTrueLogicGate = true;
				// Close ChangedToFalse logic-gate
				bChangedToFalseLogicGate = false;
				
				// And TODO ChangedToFalse logic
			}
			// Anyway, TODO WhileFalse logic
			// Do While Not Moving
			if (CanRotateInPlace())
			{
				RotateInPlaceCheck();
			}
			else
			{
				bRotateL = bRotateR = false;
			}

			if (CanTurnInPlace())
			{
				TurnInPlaceCheck(DeltaSeconds);
			}
			else
			{
				ElapsedDelayTime = 0.0f;
			}

			if (CanDynamicTransition())
			{
				DynamicTransitionCheck();
			}
		}
		
	}
	else if (Proxy->MovementState == EMovementState::MS_InAir)
	{
		// Do While InAir
		UpdateInAirValues();
	}
}

void UBaseAnimInstance::UpdateCharacterInfo(float DeltaSeconds)
{
	Gait = Proxy->Gait;
	Stance = Proxy->Stance;
	MovementState = Proxy->MovementState;
	Speed = Proxy->Speed;
	bHasMovementInput = Proxy->bHasMovementInput;

	bIsMoving = Proxy->bIsMoving;
}

void UBaseAnimInstance::UpdateAimingValues(float DeltaSeconds)
{
	// Interp the Aiming Rotation value to achieve smooth aiming rotation changes.
	// Interpolating the rotation before calculating the angle ensures the value is not affected by changes in actor rotation,
	// allowing slow aiming rotation changes with fast actor rotation changes.
	SmoothedAimingRotation = UKismetMathLibrary::RInterpTo(SmoothedAimingRotation, Proxy->AimingRotation, DeltaSeconds, SmoothedAimingRotationInterpSpeed);

	// Calculate the Aiming angle and Smoothed Aiming Angle by getting the delta between
	// the aiming rotation and the actor rotation.
	FRotator DeltaAimingRotation = UKismetMathLibrary::NormalizedDeltaRotator(Proxy->AimingRotation, Proxy->ActorRotation);
	AimingAngle = UKismetMathLibrary::MakeVector2D(DeltaAimingRotation.Yaw, DeltaAimingRotation.Pitch);
	FRotator DeltaSmoothedAimingRotation = UKismetMathLibrary::NormalizedDeltaRotator(SmoothedAimingRotation, Proxy->ActorRotation);
	SmoothedAimingAngle = UKismetMathLibrary::MakeVector2D(DeltaSmoothedAimingRotation.Yaw, DeltaSmoothedAimingRotation.Pitch);

	// Clamp the Aiming Pitch Angle to a range of 1 to 0 for use in the vertical aim sweeps.
	if (Proxy->RotationMode == ERotationMode::RM_Looking || Proxy->RotationMode == ERotationMode::RM_Aiming)
	{
		AimSweepTime = UKismetMathLibrary::MapRangeClamped(AimingAngle.Y, -90.0f, 90.0f, 1.0f, 0.0f);
		// Use the Aiming Yaw Angle divided by the number of spine+pelvis bones to
		// get the amount of spine rotation needed to remain facing the camera direction.
		SpineRotation.Yaw = AimingAngle.X / 4.0f;
	}

	// Get the delta between the Movement Input rotation and Actor rotation and map it to a range of 0-1.
	// This value is used in the aim offset behavior to make the character look toward the Movement Input.
	// Ignore the RotationMode == Velocity

	// Separate the Aiming Yaw Angle into 3 separate Yaw Times.
	// These 3 values are used in the Aim Offset behavior to improve the blending of
	// the aim offset when rotating completely around the character.
	// This allows you to keep the aiming responsive but still smoothly blend from left to right or right to left.
	LeftYawTime = UKismetMathLibrary::MapRangeClamped(FMath::Abs(SmoothedAimingAngle.X), 0.0f, 180.0f, 0.5f, 0.0f);
	RightYawTime = UKismetMathLibrary::MapRangeClamped(FMath::Abs(SmoothedAimingAngle.X), 0.0f, 180.0f, 0.5f, 1.0f);
	ForwardYawTime = UKismetMathLibrary::MapRangeClamped(SmoothedAimingAngle.X, -180.0f, 180.0f, 0.0f, 1.0f);
}

void UBaseAnimInstance::UpdateLayerValues(float DeltaSeconds)
{
	// Get the Aim Offset weight by getting the opposite of the Aim Offset Mask.
    EnableAimOffset = UKismetMathLibrary::Lerp(1.0f, 0.0f, GetAnimCurveCompact(FName("Mask_AimOffset")));

	// Set the Base Pose weights
	BasePoseN = GetAnimCurveCompact(FName("BasePose_N"));
	BasePoseCLF = GetAnimCurveCompact(FName("BasePose_CLF"));

	// Set the Additive amount weights for each body part
	SpineAdd = GetAnimCurveCompact(FName("Layering_Spine_Add"));
	HeadAdd = GetAnimCurveCompact(FName("Layering_Head_Add"));
	ArmLAdd = GetAnimCurveCompact(FName("Layering_Arm_L_Add"));
	ArmRAdd = GetAnimCurveCompact(FName("Layering_Arm_R_Add"));

	// Set the Hand Override weights
	HandR = GetAnimCurveCompact(FName("Layering_Hand_R"));
	HandL = GetAnimCurveCompact(FName("Layering_Hand_L"));

	// Blend and set the Hand IK weights to ensure they only are weighted if allowed by the Arm layers.
	EnableHandIKL = UKismetMathLibrary::Lerp(0.0f, GetAnimCurveCompact(FName("Enable_HandIK_L")), GetAnimCurveCompact(FName("Layering_Arm_L")));
	EnableHandIKR = UKismetMathLibrary::Lerp(0.0f, GetAnimCurveCompact(FName("Enable_HandIK_R")), GetAnimCurveCompact(FName("Layering_Arm_R")));

	// Set whether the arms should blend in mesh space or local space.
	// The Mesh space weight will always be 1 unless the Local Space (LS) curve is fully weighted.
	Arm_L_LS = GetAnimCurveCompact(FName("Layering_Arm_L_LS"));
	Arm_R_LS = GetAnimCurveCompact(FName("Layering_Arm_R_LS"));
	Arm_L_MS = (float)(1 - FMath::FloorToInt(Arm_L_LS));
	Arm_R_MS = (float)(1 - FMath::FloorToInt(Arm_R_LS));
}

void UBaseAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	// Locking left foot
	SetFootLocking("Enable_FootIK_L", "FootLock_L", "ik_foot_l", FootLockLAlpha, FootLockLLocation, FootLockLRotation, DeltaSeconds);
	// Locking right foot
	SetFootLocking("Enable_FootIK_R", "FootLock_R", "ik_foot_r", FootLockRAlpha, FootLockRLocation, FootLockRRotation, DeltaSeconds);
	
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetRTarget = FVector::ZeroVector;
	if (Proxy->MovementState == EMovementState::MS_None || Proxy->MovementState == EMovementState::MS_Grounded)
	{
		// Calculate left foot offset
		SetFootOffsets("Enable_FootIK_L", "ik_foot_l", "root", FootOffsetLTarget, FootOffsetLLocation, FootOffsetLRotation, DeltaSeconds);
		// Calculate right foot offset
		SetFootOffsets("Enable_FootIK_R", "ik_foot_r", "root", FootOffsetRTarget, FootOffsetRLocation, FootOffsetRRotation, DeltaSeconds);
		// Calculate pelvis offset
		SetPelvisIKOffset(FootOffsetLTarget, FootOffsetRTarget, DeltaSeconds);
	}
}

bool UBaseAnimInstance::ShouldMoveCheck()
{
	// UE_LOG(LogTemp, Warning, TEXT("IsMoving: %s, HasMovementInput: %s, Speed: %f"), *UKismetStringLibrary::Conv_BoolToString(Proxy.bIsMoving), *UKismetStringLibrary::Conv_BoolToString(Proxy.bHasMovementInput), Proxy.Speed);
	return ((Proxy->bIsMoving && Proxy->bHasMovementInput) || Proxy->Speed > 150.0f);
}

void UBaseAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	// Interp and set the Velocity Blend.
	FVelocityBlend VelocityBlendTarget = CalculateVelocityBlend();
	VelocityBlend = InterpVelocityBlend(VelocityBlend, VelocityBlendTarget, VelocityBlendInterpSpeed, DeltaSeconds);

	// Set the Diagonal Scale Amount.
	DiagonalScaleAmount = CalculateDiagonalScaleAmount();

	// Set the Relative Acceleration Amount and Interp the Lean Amount.
	RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();
	FLeanAmount TargetLeanAmount;
	TargetLeanAmount.LR = RelativeAccelerationAmount.Y;
	TargetLeanAmount.FB = RelativeAccelerationAmount.X;
	LeanAmount = InterpLeanAmount(LeanAmount, TargetLeanAmount, GroundedLeanInterpSpeed, DeltaSeconds);

	// Set the Walk Run Blend
	WalkRunBlend = CalculateWalkRunBlend();
	// Set the Stride Blend
	StrideBlend = CalculateStrideBlend();
	// Set the Standing and Crouching Play Rates
	StandingPlayRate = CalculateStandingPlayRate();
	CrouchingPlayRate = CalculateCrouchingPlayRate();
}

FVelocityBlend UBaseAnimInstance::CalculateVelocityBlend() const
{
	// Normalize character velocity and rotate it back to world forward direction.
	FVector NormalizedVelocity = Proxy->Velocity.GetSafeNormal(0.1f);
	FVector LocRelativeVelocityDir = Proxy->ActorRotation.UnrotateVector(NormalizedVelocity);

	// Map diagonals vector from 1.0 to 0.5
	float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) + FMath::Abs(LocRelativeVelocityDir.Z);
	FVector RelativeDirection = LocRelativeVelocityDir / Sum;

	// Map to VelocityBlend
	FVelocityBlend LocVelocityBlend;
	LocVelocityBlend.F = FMath::Clamp(RelativeDirection.X, 0.0f, 1.0f);
	LocVelocityBlend.B = FMath::Abs(FMath::Clamp(RelativeDirection.X, -1.0f, 0.0f));
	LocVelocityBlend.L = FMath::Abs(FMath::Clamp(RelativeDirection.Y, -1.0f, 0.0f));
	LocVelocityBlend.R = FMath::Clamp(RelativeDirection.Y, 0.0f, 1.0f);

	return LocVelocityBlend;
}

FVelocityBlend UBaseAnimInstance::InterpVelocityBlend(FVelocityBlend Current,
                                                      FVelocityBlend Target,
                                                      float InterpSpeed, float DeltaTime) const
{
	// Make VelocityBlend
	FVelocityBlend LocVelocityBlend;
	LocVelocityBlend.F = FMath::FInterpTo(Current.F, Target.F, DeltaTime, InterpSpeed);
	LocVelocityBlend.B = FMath::FInterpTo(Current.B, Target.B, DeltaTime, InterpSpeed);
	LocVelocityBlend.L = FMath::FInterpTo(Current.L, Target.L, DeltaTime, InterpSpeed);
	LocVelocityBlend.R = FMath::FInterpTo(Current.R, Target.R, DeltaTime, InterpSpeed);

	return LocVelocityBlend;
}

float UBaseAnimInstance::CalculateDiagonalScaleAmount() const
{
	check(DiagonalScaleAmountCurve);
	
	float InTime = FMath::Abs(VelocityBlend.F + VelocityBlend.B);
	return DiagonalScaleAmountCurve->GetFloatValue(InTime);
}

FVector UBaseAnimInstance::CalculateRelativeAccelerationAmount() const
{
	if (FVector::DotProduct(Proxy->PhysicalAcceleration, Proxy->Velocity) > 0.0f)
	{
		FVector ClampedAcceleration = Proxy->PhysicalAcceleration.GetClampedToMaxSize(Proxy->MaxMovementInput);
		return Proxy->ActorRotation.UnrotateVector(ClampedAcceleration / Proxy->MaxMovementInput);
	}
	else
	{
		FVector ClampedDeceleration = Proxy->PhysicalAcceleration.GetClampedToMaxSize(Proxy->MaxBrakingDeceleration);
		return Proxy->ActorRotation.UnrotateVector(ClampedDeceleration / Proxy->MaxBrakingDeceleration);
	}
}

FLeanAmount UBaseAnimInstance::InterpLeanAmount(FLeanAmount Current,
	                                            FLeanAmount Target,
	                                            float InterpSpeed, float DeltaTime) const
{
	FLeanAmount LocLeanAmount;
	LocLeanAmount.LR = FMath::FInterpTo(Current.LR, Target.LR, DeltaTime, InterpSpeed);
	LocLeanAmount.FB = FMath::FInterpTo(Current.FB, Target.FB, DeltaTime, InterpSpeed);
	return LocLeanAmount;
}

float UBaseAnimInstance::CalculateWalkRunBlend() const
{
	if (Proxy->Gait == EGaitState::GS_Walking)
	{
		return 0.0f;
	}
	// Gait == Running or Sprinting
	return 1.0f;
}

float UBaseAnimInstance::CalculateStrideBlend() const
{
	check(StrideBlendNWalk && StrideBlendNRun && StrideBlendCWalk);

	// Get walk/run stride in current speed
	float StandingWalkStride = StrideBlendNWalk->GetFloatValue(Proxy->Speed);
	float StandingRunStride = StrideBlendNRun->GetFloatValue(Proxy->Speed);
	// Get crouch stride in current speed
	float CrouchingStride = StrideBlendCWalk->GetFloatValue(Proxy->Speed);

	// Get walk/run's current weight
	float WalkRunGaitWeight = GetAnimCurveClamped(FName("Weight_Gait"), -1.0f, 0.0f, 1.0f);
	
	// Blend walk/run stride based on gait weight
	float WalkRunStrideBlend = UKismetMathLibrary::Lerp(StandingWalkStride, StandingRunStride, WalkRunGaitWeight);

	// Get crouching weight
	float CrouchingStanceWeight = GetCurveValue(FName("BasePose_CLF"));

	// Blend blended-walkrun stride with crouch stride
	float FinalStrideBlend = UKismetMathLibrary::Lerp(WalkRunStrideBlend, CrouchingStride, CrouchingStanceWeight);

	return FinalStrideBlend;
}

float UBaseAnimInstance::GetAnimCurveClamped(FName Name, float Bias, float ClampMin, float ClampMax) const
{
	return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
}

float UBaseAnimInstance::GetAnimCurveCompact(FName Name) const
{
	return GetCurveValue(Name);
}

float UBaseAnimInstance::CalculateStandingPlayRate() const
{
	// Calculate current speed in different gait's animation speed rate.
	float WalkSpeedRate = Proxy->Speed / AnimatedWalkSpeed;
	float RunSpeedRate = Proxy->Speed / AnimatedRunSpeed;
	float SprintSpeedRate = Proxy->Speed / AnimatedSprintSpeed;

	// Weight_Gait in Walk Anima == 1, Run Anim == 2, Sprint Anim == 3
	float WalkRunGaitWeight = GetAnimCurveClamped(FName("Weight_Gait"), -1.0f, 0.0f, 1.0f);
	float WalkRunSpeedRate = UKismetMathLibrary::Lerp(WalkSpeedRate, RunSpeedRate, WalkRunGaitWeight);
	float RunSprintGaitWeight = GetAnimCurveClamped(FName("Weight_Gait"), -2.0f, 0.0f, 1.0f);
	float RunSprintSpeedRate = UKismetMathLibrary::Lerp(WalkRunSpeedRate, SprintSpeedRate, RunSprintGaitWeight);

	// Keep sync with stride blend rate
	float FinalSpeedRate = RunSprintSpeedRate / StrideBlend;
	// scale the value by world z scale
	FinalSpeedRate /= GetOwningComponent()->GetComponentScale().Z;
	FinalSpeedRate = FMath::Clamp<float>(FinalSpeedRate, 0.0f, 3.0f);

	return FinalSpeedRate;
}

float UBaseAnimInstance::CalculateCrouchingPlayRate() const
{
	float Result = FMath::Clamp(((Proxy->Speed / AnimatedCrouchSpeed) / StrideBlend) / GetOwningComponent()->GetComponentScale().Z, 0.0f, 2.0f);
	return Result;
}

void UBaseAnimInstance::UpdateRotationValues()
{
	// Set the Movement Direction
	MovementDirection = CalculateMovementDirection();

	// Set the Yaw Offsets.
	// These values influence the "YawOffset" curve in the animgraph and are used to
	// offset the characters rotation for more natural movement.
	// The curves allow for fine control over how the offset behaves for each movement direction.
	check(YawOffsetFB && YawOffsetLR);
	float DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(Proxy->Velocity.ToOrientationRotator(), Proxy->AimingRotation).Yaw;
	FYaw = YawOffsetFB->GetVectorValue(DeltaYaw).X;
	BYaw = YawOffsetFB->GetVectorValue(DeltaYaw).Y;
	LYaw = YawOffsetLR->GetVectorValue(DeltaYaw).X;
	RYaw = YawOffsetLR->GetVectorValue(DeltaYaw).Y;
}

EMovementDirection UBaseAnimInstance::CalculateMovementDirection() const
{
	if (Proxy->Gait == EGaitState::GS_Sprinting)
	{
		return EMovementDirection::MD_Forward;
	}
	// Gait == Walking or Running
	// RotationMode == Looking or Aiming
	float Angle = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(Proxy->Velocity), Proxy->AimingRotation).Yaw;
	return CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Angle);
}

EMovementDirection UBaseAnimInstance::CalculateQuadrant(EMovementDirection Current,
	                                                    float FRThreshold, float FLThreshold,
	                                                    float BRThreshold, float BLThreshold,
	                                                    float Buffer, float Angle) const
{
	// Calculate forward direction
	bool IncreaseBuffer = Current != EMovementDirection::MD_Forward || Current != EMovementDirection::MD_Backward;
	if (AngleInRange(Angle, FLThreshold, FRThreshold, Buffer, IncreaseBuffer))
	{
		return EMovementDirection::MD_Forward;
	}

	// Calculate right direction
	IncreaseBuffer = Current != EMovementDirection::MD_Right || Current != EMovementDirection::MD_Left;
	if (AngleInRange(Angle, FRThreshold, BRThreshold, Buffer, IncreaseBuffer))
	{
		return EMovementDirection::MD_Right;
	}

	// Calculate left direction
	if (AngleInRange(Angle, BLThreshold, FLThreshold, Buffer, IncreaseBuffer))
	{
		return EMovementDirection::MD_Left;
	}
	
	return EMovementDirection::MD_Backward;
}

bool UBaseAnimInstance::AngleInRange(float Angle,
								     float MinAngle, float MaxAngle,
								     float Buffer, bool bIncreaseBuffer) const
{
	if (bIncreaseBuffer)
	{
		return UKismetMathLibrary::InRange_FloatFloat(Angle, MinAngle - Buffer, MaxAngle + Buffer, true, true);
	}
	else
	{
		return UKismetMathLibrary::InRange_FloatFloat(Angle, MinAngle + Buffer, MaxAngle - Buffer, true, true);
	}
}

bool UBaseAnimInstance::CanRotateInPlace() const
{
	return Proxy->RotationMode == ERotationMode::RM_Aiming;
}

bool UBaseAnimInstance::CanTurnInPlace() const
{
	return Proxy->RotationMode == ERotationMode::RM_Looking && GetCurveValue(FName("Enable_Transition")) > 0.99f;
}

bool UBaseAnimInstance::CanDynamicTransition() const
{
	return GetCurveValue(FName("Enable_Transition")) == 1.0f;
}

void UBaseAnimInstance::RotateInPlaceCheck()
{
	// Step 1: Check if the character should rotate left or right by checking if the
	// Aiming Angle exceeds the threshold.
	bRotateL = AimingAngle.X < RotateMinThreshold;
	bRotateR = AimingAngle.X > RotateMaxThreshold;

	// Step 2: If the character should be rotating, set the Rotate Rate to scale with the Aim Yaw Rate.
	// This makes the character rotate faster when moving the camera faster.
	if (bRotateL || bRotateR)
	{
		RotateRate = UKismetMathLibrary::MapRangeClamped(Proxy->AimYawRate,
		                                              AimYawRateMinRange, AimYawRateMaxRange,
		                                              MinPlayRate, MaxPlayRate);
	}
}

void UBaseAnimInstance::TurnInPlaceCheck(float DeltaTime)
{
	// Step 1: Check if Aiming angle is outside of the Turn Check Min Angle,
	// and if the Aim Yaw Rate is below the Aim Yaw Rate Limit.
	// If so, begin counting the Elapsed Delay Time. If not, reset the Elapsed Delay Time.
	// This ensures the conditions remain true for a sustained peroid of time before turning in place.
	if (FMath::Abs(AimingAngle.X) > TurnCheckMinAngle && Proxy->AimYawRate < AimYawRateLimit)
	{
		ElapsedDelayTime += DeltaTime;
		// Step 2: Check if the Elapsed Delay time exceeds the set delay (mapped to the turn angle range).
		// If so, trigger a Turn In Place.
		float MappedAimingAngleX = UKismetMathLibrary::MapRangeClamped(FMath::Abs(AimingAngle.X), TurnCheckMinAngle, 180.0f, MinAngleDelay, MaxAngleDelay);
		if (ElapsedDelayTime > MappedAimingAngleX)
		{
			FRotator TargetRotation(0.0f, Proxy->AimingRotation.Yaw, 0.0f);
			TurnInPlace(TargetRotation, 1.0f, 0.0f, false);
		}
	}
	else
	{
		ElapsedDelayTime = 0.0f;
	}
}

void UBaseAnimInstance::DynamicTransitionCheck()
{
}

// TODO: 0 Finish Turn In Place
void UBaseAnimInstance::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool bOverrideCurrent)
{
	// Step 1: Set Turn Angle
	float TurnAngle = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, Proxy->ActorRotation).Yaw;

	// Step 2: Choose Turn Asset based on the Turn Angle and Stance
	FTurnInPlaceAsset TargetTurnAsset;
	if (FMath::Abs(TurnAngle) < Turn180Threshold)
	{
		if (TurnAngle < 0.0f)
		{

		}
		else
		{
			
		}
	}
	else
	{
		
	}
	
}

void UBaseAnimInstance::SetFootLocking(FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
	float& CurrentFootLockAlpha, FVector& CurrentFootLockLocation, FRotator& CurrentFootLockRotation, float DeltaSeconds)
{
	// Only update values if FootIK curve has a weight.
	if (GetCurveValue(EnableFootIKCurve) <= 0.0f) return;

	// Step 1: Set Local FootLock Curve value
	float FootLockCurveValue = GetCurveValue(FootLockCurve);

	//UE_LOG(LogTemp, Warning, TEXT("%s FootLockCurveValue: %f"), *FootLockCurve.ToString(), FootLockCurveValue);
	//UE_LOG(LogTemp, Warning, TEXT("%s CurrentFootLockAlpha: %f"), *FootLockCurve.ToString(), CurrentFootLockAlpha);

	// Step 2: Only update the FootLock Alpha if the new value is less than the current,
	// or it equals 1. This makes it so that the foot can only blend out of the locked position
	// or lock to a new position, and never blend in.
	if (FootLockCurveValue >= 0.99f || FootLockCurveValue < CurrentFootLockAlpha)
	{
		CurrentFootLockAlpha = FootLockCurveValue;
		//UE_LOG(LogTemp, Warning, TEXT("%s New CurrentFootLockAlpha: %f"), *FootLockCurve.ToString(), CurrentFootLockAlpha);
	}

	// Step 3: If the Foot Lock curve equals 1,
	// save the new lock location and rotation in component space.
	if (CurrentFootLockAlpha >= 0.99f)
	{
		FTransform FootBoneTransform = GetOwningComponent()->GetSocketTransform(IKFootBone, RTS_Component);
		CurrentFootLockLocation = FootBoneTransform.GetLocation();
		CurrentFootLockRotation = FootBoneTransform.Rotator();
	}

	// Step 4: If the Foot Lock Alpha has a weight,
	// update the Foot Lock offsets to keep the foot planted in place while the capsule moves.
	if (CurrentFootLockAlpha > 0.0f)
	{
		SetFootLockOffsets(CurrentFootLockLocation, CurrentFootLockRotation, DeltaSeconds);
	}
}

void UBaseAnimInstance::SetFootLockOffsets(FVector& LocalLocation, FRotator& LocalRotation, float DeltaSeconds)
{
	// Use the delta between the current and last updated rotation
	// to find how much the foot should be rotated to remain planted on the ground.
	FRotator RotationDifference;
	if (Proxy->bIsMovingOnGround)
	{
		RotationDifference = UKismetMathLibrary::NormalizedDeltaRotator(Proxy->ActorRotation, Proxy->LastUpdateRotation);
	}

	// Get the distance traveled between frames relative to the mesh rotation
	// to find how much the foot should be offset to remain planted on the ground.
	// Get component's world rotation
	FVector LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector(Proxy->Velocity * DeltaSeconds);
	
	// Subtract the location difference from the current local location and rotate it
	// by the rotation difference to keep the foot planted in component space.
	LocalLocation = UKismetMathLibrary::RotateAngleAxis(LocalLocation - LocationDifference, RotationDifference.Yaw, FVector(0.0f, 0.0f, -1.0f));

	// Subtract the Rotation Difference from the current Local Rotation to get the new local rotation.
	LocalRotation = UKismetMathLibrary::NormalizedDeltaRotator(LocalRotation, RotationDifference);
}

void UBaseAnimInstance::SetFootOffsets(FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
	FVector& CurrentLocationTarget, FVector& CurrentLocationOffset, FRotator& CurrentRotationOffset, float DeltaSeconds)
{
	// Only update Foot IK offset values if the Foot IK curve has a weight.
	// If it equals 0, clear the offset values.
	if (GetCurveValue(EnableFootIKCurve) > 0.0f)
	{
		// Step 1: Trace downward from the foot location to find the geometry.
		// If the surface is walkable, save the Impact Location and Normal.
		FVector FootBoneLocation = GetOwningComponent()->GetSocketLocation(IKFootBone);
		FVector RootBoneLocation = GetOwningComponent()->GetSocketLocation(RootBone);
		FVector IKFootFloorLocation(FootBoneLocation.X, FootBoneLocation.Y, RootBoneLocation.Z);

		FVector LineTraceStartLocation = IKFootFloorLocation + FVector(0.0f, 0.0f, IKTraceDistanceAboveFoot);
		FVector LineTraceEndLocation = IKFootFloorLocation - FVector(0.0f, 0.0f, IKTraceDistanceBelowFoot);

		FRotator TargetRotationOffset;
		
		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore;
		FVector ImpactPoint;
		FVector ImpactNormal;
		// UKismetSystemLibrary::LineTraceSingle(Proxy.Character, LineTraceStartLocation, LineTraceEndLocation, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForOneFrame, HitResult, true);
		FCollisionQueryParams QueryParams;
		QueryParams.bTraceComplex = false;
		QueryParams.AddIgnoredActor(Proxy->Character);
		GetWorld()->LineTraceSingleByChannel(HitResult, LineTraceStartLocation, LineTraceEndLocation, ECC_Visibility, QueryParams);
		if (Proxy->MovementComponent->IsWalkable(HitResult))
		{
			ImpactPoint = HitResult.ImpactPoint;
			ImpactNormal = HitResult.ImpactNormal;

			// Step 1.1: Find the difference in location from the Impact point and
			// the expected (flat) floor location. These values are offset by the nomrmal multiplied by
			// the foot height to get better behavior on angled surfaces.
			CurrentLocationTarget = (ImpactPoint + ImpactNormal * FootHeight) - (IKFootFloorLocation + FVector(0.0f, 0.0f, 1.0f) * FootHeight);

			// Step 1.2: Calculate the Rotation offset by getting the Atan2 of the Impact Normal.
			float ImpactNormalRoll = UKismetMathLibrary::DegAtan2(ImpactNormal.Y, ImpactNormal.Z);
			float ImpactNormalPitch = UKismetMathLibrary::DegAtan2(ImpactNormal.X, ImpactNormal.Z) * -1.0f;
			
			TargetRotationOffset = FRotator(ImpactNormalPitch,0.0f,ImpactNormalRoll);
		}

		// Step 2: Interp the Current Location Offset to the new target value.
		// Interpolate at different speeds based on whether the new target is above or below the current one.
		float InterpSpeed = 15.0f;
		if (CurrentLocationOffset.Z > CurrentLocationTarget.Z)
		{
			InterpSpeed = 30.0f;
		}
		CurrentLocationOffset = UKismetMathLibrary::VInterpTo(CurrentLocationOffset, CurrentLocationTarget, DeltaSeconds, InterpSpeed);

		// Step 3: Interp the Current Rotation Offset to the new target value.
		CurrentRotationOffset = UKismetMathLibrary::RInterpTo(CurrentRotationOffset, TargetRotationOffset, DeltaSeconds, 30.0f);
	}
	else
	{
		CurrentLocationOffset = FVector::ZeroVector;
		CurrentRotationOffset = FRotator::ZeroRotator;
	}
}

void UBaseAnimInstance::SetPelvisIKOffset(FVector FootOffsetLTarget, FVector FootOffsetRTarget, float DeltaSeconds)
{
	// Calculate the Pelvis Alpha by finding the average Foot IK weight.
	// If the alpha is 0, clear the offset.
	PelvisAlpha = (GetCurveValue("Enable_FootIK_L") + GetCurveValue("Enable_FootIK_R")) / 2.0f;
	if (PelvisAlpha <= 0.0f)
	{
		PelvisOffset = FVector::Zero();
		return;
	}

	// Step 1: Set the new Pelvis Target to be the lowest Foot Offset
	FVector PelvisTarget = FootOffsetLTarget.Z < FootOffsetRTarget.Z ? FootOffsetLTarget : FootOffsetRTarget;
	// Step 2: Interp the Current Pelvis Offset to the new target value.
	// Interpolate at different speeds based on whether the new target is above or below the current one.
	float InterpSpeed = 15.0f;
	if (PelvisTarget.Z > PelvisOffset.Z)
	{
		InterpSpeed = 10.0f;
	}
	PelvisOffset = UKismetMathLibrary::VInterpTo(PelvisOffset, PelvisTarget, DeltaSeconds, InterpSpeed);
}

void UBaseAnimInstance::UpdateInAirValues()
{
	// Update the fall speed. Setting this value only while in the air allows you to use it within the AnimGraph for the landing strength.
	// If not, the Z velocity would return to 0 on landing. 
	FallSpeed = Proxy->Velocity.Z;

	// Set the Land Prediction weight.
	LandPrediction = CalculateLandPrediction();
}

float UBaseAnimInstance::CalculateLandPrediction()
{
	if (FallSpeed >= -200.0f) return 0.0f;

	FHitResult HitResult;
	FVector Start = Proxy->CapsuleLocation;
	FVector ClampedVelocity = FVector(Proxy->Velocity.X, Proxy->Velocity.Y, FMath::Clamp(Proxy->Velocity.Z, -4000.0f, -200.0f));
	FVector NormalizedVelocity = ClampedVelocity.GetUnsafeNormal();
	float MappedFallSpeed = UKismetMathLibrary::MapRangeClamped(Proxy->Velocity.Z, 0.0f, -4000.0f, 50.0f, 2000.0f);
	FVector End = Start + NormalizedVelocity * MappedFallSpeed;
	FName ProfileName = "ALS_Character";
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(Proxy->Character);
	// UKismetSystemLibrary::CapsuleTraceSingleByProfile()
	GetWorld()->SweepSingleByProfile(HitResult, Start, End, FQuat::Identity, ProfileName, FCollisionShape::MakeCapsule(Proxy->CapsuleRadius, Proxy->CapsuleHalfHeight), QueryParams);
	if (Proxy->MovementComponent->IsWalkable(HitResult) && HitResult.bBlockingHit)
	{
		return UKismetMathLibrary::Lerp(LandPredictionCurve->GetFloatValue(HitResult.Time), 0.0f, GetCurveValue("Mask_LandPrediction"));
	}
	
	return 0.0f;
}
