// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAnimInstance.h"

#include "Curves/CurveVector.h"
#include "Kismet/KismetMathLibrary.h"

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

	// Test
	BasePoseN = InAnimInstance->GetCurveValue(FName("BasePose_N"));
	UE_LOG(LogTemp, Warning, TEXT("FBaseAnimInstanceProxy::PreUpdate.BasePoseN: %f"), BasePoseN);
	
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
	// Layer Blending
	EnableAimOffset = 1.0f;
	BasePoseN = 1.0f;
	EnableHandIKL = 1.0f;
	EnableHandIKR = 1.0f;
	// In Air
	LandPrediction = 1.0f;
}

void UBaseAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	
	// Only update if character is valid
	if (DeltaSeconds == 0.0f) return;
	if (Proxy.Character == nullptr) return;
	
	UpdateAimingValues(DeltaSeconds);
	UpdateLayerValues(DeltaSeconds);
	UpdateFootIK(DeltaSeconds);

	// Check Movement Mode
	if (Proxy.MovementState == EMovementState::MS_Grounded)
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
	else if (Proxy.MovementState == EMovementState::MS_InAir)
	{
		
	}
}

void UBaseAnimInstance::UpdateAimingValues(float DeltaSeconds)
{
	// Interp the Aiming Rotation value to achieve smooth aiming rotation changes.
	// Interpolating the rotation before calculating the angle ensures the value is not affected by changes in actor rotation,
	// allowing slow aiming rotation changes with fast actor rotation changes.
	SmoothedAimingRotation = UKismetMathLibrary::RInterpTo(SmoothedAimingRotation, Proxy.AimingRotation, DeltaSeconds, SmoothedAimingRotationInterpSpeed);

	// Calculate the Aiming angle and Smoothed Aiming Angle by getting the delta between
	// the aiming rotation and the actor rotation.
	FRotator DeltaAimingRotation = UKismetMathLibrary::NormalizedDeltaRotator(Proxy.AimingRotation, Proxy.ActorRotation);
	AimingAngle = UKismetMathLibrary::MakeVector2D(DeltaAimingRotation.Yaw, DeltaAimingRotation.Pitch);
	FRotator DeltaSmoothedAimingRotation = UKismetMathLibrary::NormalizedDeltaRotator(SmoothedAimingRotation, Proxy.ActorRotation);
	SmoothedAimingAngle = UKismetMathLibrary::MakeVector2D(DeltaSmoothedAimingRotation.Yaw, DeltaSmoothedAimingRotation.Pitch);

	// Clamp the Aiming Pitch Angle to a range of 1 to 0 for use in the vertical aim sweeps.
	if (Proxy.RotationMode == ERotationMode::RM_Looking || Proxy.RotationMode == ERotationMode::RM_Aiming)
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
	// BasePoseN = GetAnimCurveCompact(FName("BasePose_N"));
	BasePoseN = Proxy.BasePoseN;
	BasePoseCLF = GetAnimCurveCompact(FName("BasePose_CLF"));
	UE_LOG(LogTemp, Warning, TEXT("BasePoseN: %f"), BasePoseN);

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
}

bool UBaseAnimInstance::ShouldMoveCheck()
{
	return ((Proxy.bIsMoving && Proxy.bHasMovementInput) || Proxy.Speed > 150.0f);
}

void UBaseAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	// Interp and set the Velocity Blend.
	VelocityBlend = InterpVelocityBlend(VelocityBlend, CalculateVelocityBlend(), VelocityBlendInterpSpeed, DeltaSeconds);

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
	FVector LocRelativeVelocityDir = Proxy.ActorRotation.UnrotateVector(Proxy.Velocity.GetSafeNormal(0.1f));
	float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) + FMath::Abs(LocRelativeVelocityDir.Z);
	FVector RelativeDirection = LocRelativeVelocityDir / Sum;

	// Make VelocityBlend
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
	if (FVector::DotProduct(Proxy.PhysicalAcceleration, Proxy.Velocity) > 0.0f)
	{
		FVector ClampedAcceleration = Proxy.PhysicalAcceleration.GetClampedToMaxSize(Proxy.MaxMovementInput);
		return Proxy.ActorRotation.UnrotateVector(ClampedAcceleration / Proxy.MaxMovementInput);
	}
	else
	{
		FVector ClampedDeceleration = Proxy.PhysicalAcceleration.GetClampedToMaxSize(Proxy.MaxBrakingDeceleration);
		return Proxy.ActorRotation.UnrotateVector(ClampedDeceleration / Proxy.MaxBrakingDeceleration);
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
	if (Proxy.Gait == EGaitState::GS_Walking)
	{
		return 0.0f;
	}
	// Gait == Running or Sprinting
	return 1.0f;
}

float UBaseAnimInstance::CalculateStrideBlend() const
{
	check(StrideBlendNWalk && StrideBlendNRun && StrideBlendCWalk);

	float WeightGait = GetAnimCurveClamped(FName("Weight_Gait"), -1.0f, 0.0f, 1.0f);
	float WalkRunLerp = UKismetMathLibrary::Lerp(StrideBlendNWalk->GetFloatValue(Proxy.Speed), StrideBlendNRun->GetFloatValue(Proxy.Speed), WeightGait);

	float LocBasePoseCLF = GetCurveValue(FName("BasePose_CLF"));
	float BlendResult = UKismetMathLibrary::Lerp(WalkRunLerp, StrideBlendCWalk->GetFloatValue(Proxy.Speed), LocBasePoseCLF);

	return BlendResult;
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
	float WeightGait = GetAnimCurveClamped(FName("Weight_Gait"), -1.0f, 0.0f, 1.0f);
	float WalkRunSpeedLerp = UKismetMathLibrary::Lerp(Proxy.Speed / AnimatedWalkSpeed, Proxy.Speed / AnimatedRunSpeed, WeightGait);

	float WeightGait2 = GetAnimCurveClamped(FName("Weight_Gait"), -2.0f, 0.0f, 1.0f);
	float SprintSpeedLerp = UKismetMathLibrary::Lerp(WalkRunSpeedLerp, Proxy.Speed / AnimatedSprintSpeed, WeightGait2);

	float Result = FMath::Clamp((SprintSpeedLerp / StrideBlend) / GetOwningComponent()->GetComponentScale().Z, 0.0f, 3.0f);

	return Result;
}

float UBaseAnimInstance::CalculateCrouchingPlayRate() const
{
	float Result = FMath::Clamp(((Proxy.Speed / AnimatedCrouchSpeed) / StrideBlend) / GetOwningComponent()->GetComponentScale().Z, 0.0f, 2.0f);
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
	float DeltaYaw = UKismetMathLibrary::NormalizedDeltaRotator(Proxy.Velocity.ToOrientationRotator(), Proxy.AimingRotation).Yaw;
	FYaw = YawOffsetFB->GetVectorValue(DeltaYaw).X;
	BYaw = YawOffsetFB->GetVectorValue(DeltaYaw).Y;
	LYaw = YawOffsetLR->GetVectorValue(DeltaYaw).X;
	RYaw = YawOffsetLR->GetVectorValue(DeltaYaw).Y;
}

EMovementDirection UBaseAnimInstance::CalculateMovementDirection() const
{
	if (Proxy.Gait == EGaitState::GS_Sprinting)
	{
		return EMovementDirection::MD_Forward;
	}
	// Gait == Walking or Running
	// RotationMode == Looking or Aiming
	float Angle = UKismetMathLibrary::NormalizedDeltaRotator(Proxy.Velocity.ToOrientationRotator(), Proxy.AimingRotation).Yaw;
	return CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Angle);
}

EMovementDirection UBaseAnimInstance::CalculateQuadrant(EMovementDirection Current,
	                                                    float FRThreshold, float FLThreshold,
	                                                    float BRThreshold, float BLThreshold,
	                                                    float Buffer, float Angle) const
{
	if (AngleInRange(Angle, FLThreshold, FRThreshold, Buffer,
		Current != EMovementDirection::MD_Forward || Current != EMovementDirection::MD_Backward))
	{
		return EMovementDirection::MD_Forward;
	}
	else if (AngleInRange(Angle, FRThreshold, BRThreshold, Buffer,
		Current != EMovementDirection::MD_Right || Current != EMovementDirection::MD_Left))
	{
		return EMovementDirection::MD_Right;
	}
	else if (AngleInRange(Angle, BLThreshold, FLThreshold, Buffer,
		Current != EMovementDirection::MD_Right || Current != EMovementDirection::MD_Left))
	{
		return EMovementDirection::MD_Left;
	}
	else
	{
		return EMovementDirection::MD_Backward;
	}
}

bool UBaseAnimInstance::AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer,
	bool bIncreaseBuffer) const
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
	return Proxy.RotationMode == ERotationMode::RM_Aiming;
}

bool UBaseAnimInstance::CanTurnInPlace() const
{
	return Proxy.RotationMode == ERotationMode::RM_Looking && GetCurveValue(FName("Enable_Transition")) > 0.99f;
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
		RotateRate = UKismetMathLibrary::MapRangeClamped(Proxy.AimYawRate,
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
	if (FMath::Abs(AimingAngle.X) > TurnCheckMinAngle && Proxy.AimYawRate < AimYawRateLimit)
	{
		ElapsedDelayTime += DeltaTime;
		// Step 2: Check if the Elapsed Delay time exceeds the set delay (mapped to the turn angle range).
		// If so, trigger a Turn In Place.
		float MappedAimingAngleX = UKismetMathLibrary::MapRangeClamped(FMath::Abs(AimingAngle.X), TurnCheckMinAngle, 180.0f, MinAngleDelay, MaxAngleDelay);
		if (ElapsedDelayTime > MappedAimingAngleX)
		{
			FRotator TargetRotation(0.0f, Proxy.AimingRotation.Yaw, 0.0f);
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
	float TurnAngle = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, Proxy.ActorRotation).Yaw;

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
