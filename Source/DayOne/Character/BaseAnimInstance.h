// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "BaseAnimInstance.generated.h"

/**
 * 
 */
USTRUCT()
struct FBaseAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

	FBaseAnimInstanceProxy();
	FBaseAnimInstanceProxy(UAnimInstance* Instance);

	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	// virtual void Update(float DeltaSeconds) override;
	// virtual void PostUpdate(UAnimInstance* InAnimInstance) const override;

	// Update functions
	// Get Information from the Character via the Character Interface to use throughout the AnimBP and AnimGraph.
	void UpdateCharacterInfo();
	
	UPROPERTY(Transient)
	class ABaseCharacter* Character;
	UPROPERTY(Transient)
	class ULocomotionComponent* MovementComponent;
	
	// Essentials variables
	FVector Velocity;
	FVector PhysicalAcceleration;
	FVector MovementInput;
	bool bIsMoving;
	bool bIsMovingOnGround;
	bool bHasMovementInput;
	float Speed;
	float MovementInputAmount;
	FRotator AimingRotation;
	float AimYawRate;
	FRotator ActorRotation;
	FRotator LastUpdateRotation;
	float MaxMovementInput;
	float MaxBrakingDeceleration;
	bool bIsWalkable;

	// Character capsule info.
	// Capsule's world location;
	FVector CapsuleLocation;
	float CapsuleRadius;
	float CapsuleHalfHeight;

	// Current State variables
	EMovementState MovementState;
	EMovementState PrevMovementState;
	EMovementAction MovementAction;
	ERotationMode RotationMode;
	EGaitState Gait;
	EStanceState Stance;
};

USTRUCT(BlueprintType, meta=(ScriptName="VelocityBlend"))
struct FVelocityBlend
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Forward")
	float F;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Backward")
	float B;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Left")
	float L;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Right")
	float R;
};

USTRUCT()
struct FLeanAmount
{
	GENERATED_BODY()

	float LR;
	float FB;
};

UENUM(BlueprintType, meta=(ScriptName="MovementDirection"))
enum class EMovementDirection : uint8
{
	MD_Forward = 0 UMETA(DisplayName = "Forward"),
	MD_Right UMETA(DisplayName = "Right"),
	MD_Left UMETA(DisplayName = "Left"),
	MD_Backward UMETA(DisplayName = "Backward"),
	MD_MAX
};

USTRUCT(BlueprintType)
struct FTurnInPlaceAsset : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimSequence* Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimatedAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bScaleTurnAngle;
};

UCLASS()
class DAYONE_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	
public:
	friend struct FBaseAnimInstanceProxy;

protected:
	FBaseAnimInstanceProxy* Proxy;
	
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override
	{
		Proxy = new FBaseAnimInstanceProxy(this);
		return Proxy;
	}
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override
	{
		check(InProxy == Proxy);
		delete InProxy;
	}

	// Update functions
	void UpdateCharacterInfo(float DeltaSeconds);
	void UpdateAimingValues(float DeltaSeconds);
	void UpdateLayerValues(float DeltaSeconds);
	void UpdateFootIK(float DeltaSeconds);

	// Variables
	// Foot IK
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float FootLockLAlpha;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FVector FootLockLLocation;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FRotator FootLockLRotation;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float FootLockRAlpha;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FVector FootLockRLocation;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FRotator FootLockRRotation;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FVector FootOffsetLLocation;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FRotator FootOffsetLRotation;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FVector FootOffsetRLocation;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FRotator FootOffsetRRotation;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float PelvisAlpha;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	FVector PelvisOffset;
	
	// Character Info
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	EGaitState Gait;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	EStanceState Stance;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	EMovementState MovementState;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float Speed;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	bool bHasMovementInput;
	// InAir values
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float LandPrediction;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float FallSpeed;
	// Layer Blending values
	float EnableAimOffset;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BasePoseN;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float BasePoseCLF;
	float SpineAdd;
	float HeadAdd;
	float ArmLAdd;
	float ArmRAdd;
	float HandL;
	float HandR;
	float EnableHandIKL;
	float EnableHandIKR;
	float Arm_L_LS;
	float Arm_R_LS;
	float Arm_L_MS;
	float Arm_R_MS;
	// Aiming values
	FRotator SmoothedAimingRotation;
	FVector2d AimingAngle;
	FVector2d SmoothedAimingAngle;
	float AimSweepTime;
	FRotator SpineRotation;
	float LeftYawTime;
	float RightYawTime;
	float ForwardYawTime;
	// Grounded values;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bShouldMove;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bIsMoving;
	bool bRotateL;
	bool bRotateR;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVelocityBlend VelocityBlend;
	float DiagonalScaleAmount;
	FVector RelativeAccelerationAmount;
	FLeanAmount LeanAmount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float WalkRunBlend;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float StrideBlend;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float StandingPlayRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CrouchingPlayRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	EMovementDirection MovementDirection;
	float FYaw;
	float BYaw;
	float LYaw;
	float RYaw;
	float RotateRate;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float RotationScale;
	// TurnInPlace values
	float ElapsedDelayTime;
	float TurnCheckMinAngle;
	float AimYawRateLimit;
	float MinAngleDelay;
	float MaxAngleDelay;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	//FTurnInPlaceAsset* N_TurnIP_L90;
	// RotateInPlace values
	float RotateMinThreshold;
	float RotateMaxThreshold;
	float AimYawRateMinRange;
	float AimYawRateMaxRange;
	float MinPlayRate;
	float MaxPlayRate;
	float Turn180Threshold;
	// Config
	float SmoothedAimingRotationInterpSpeed;
	float VelocityBlendInterpSpeed;
	float GroundedLeanInterpSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AnimatedWalkSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AnimatedRunSpeed;
	float AnimatedSprintSpeed;
	float AnimatedCrouchSpeed;
	float IKTraceDistanceAboveFoot;
	float IKTraceDistanceBelowFoot;
	float FootHeight;
	// Blend Curves
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveFloat* DiagonalScaleAmountCurve;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveFloat* StrideBlendNWalk;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveFloat* StrideBlendNRun;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveFloat* StrideBlendCWalk;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveVector* YawOffsetFB;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveVector* YawOffsetLR;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveFloat* LandPredictionCurve;
	
private:
	// Enable Movement Animations if IsMoving and HasMovementInput,
	// or if the Speed is greater than 150. 
	bool ShouldMoveCheck();

	void UpdateMovementValues(float DeltaSeconds);
	// Calculate the Velocity Blend.
	// This value represents the velocity amount of the actor in each direction (normalized so that diagonals equal .5 for each direction),
	// and is used in a BlendMulti node to produce better directional blending than a standard BlendSpace.
	FVelocityBlend CalculateVelocityBlend() const;
	FVelocityBlend InterpVelocityBlend(FVelocityBlend Current, FVelocityBlend Target, float InterpSpeed, float DeltaTime) const;
	// Calculate the Diagnal Scale Amount.
	// This value is used to scale the Foot IK Root bone to make the Foot IK bones cover more distance
	// on the diagonal blends.
	// Without scaling, the feet would not move far enough on the diagonal direction due to the linear
	// translational blending of the IK bones.
	// The curve is used to easily map the value.
	float CalculateDiagonalScaleAmount() const;
	// Calculate the Relative Acceleration Amount.
	// This value represents the current amount of acceleration / deceleration relative to the actor rotation.
	// It is normalized to a range of -1 to 1 so that -1 equals the Max Braking Deceleration,
	// and 1 equals the Max Acceleration of the Character Movement Component.
	FVector CalculateRelativeAccelerationAmount() const;
	FLeanAmount InterpLeanAmount(FLeanAmount Current, FLeanAmount Target, float InterpSpeed, float DeltaTime) const;
	// Calculate the Walk Run Blend.
	// This value is used within the Blendspaces to blend between walking and running.
	float CalculateWalkRunBlend() const;
	// Calculate the Stride Blend.
	// This value is used within the blendspaces to scale the stride (distance feet travel)
	// so that the character can walk or run at different movement speeds.
	// It also allows the walk or run gait animations to blend independently while
	// still matching the animation speed to the movement speed,
	// preventing the character from needing to play a half walk+half run blend.
	// The curves are used to map the stride amount to the speed for maximum control.
	float CalculateStrideBlend() const;
	float GetAnimCurveClamped(FName Name, float Bias, float ClampMin, float ClampMax) const;
	float GetAnimCurveCompact(FName Name) const;
	// Calculate the Play Rate by dividing the Character's speed by the Animated Speed for each gait.
	// The lerps are determined by the "Weight_Gait" anim curve that exists on every locomotion cycle
	// so that the play rate is always in sync with the currently blended animation.
	// The value is also divided by the Stride Blend and the mesh scale
	// so that the play rate increases as the stride or scale gets smaller.
	float CalculateStandingPlayRate() const;
	// Calculate the Crouching Play Rate by dividing the Character's speed by the Animated Speed.
	// This value needs to be separate from the standing play rate to improve the blend from crocuh
	// to stand while in motion.
	float CalculateCrouchingPlayRate() const;
	
	void UpdateRotationValues();
	// Calculate the Movement Direction.
	// This value represents the direction the character is moving relative to the camera during
	// the Looking Cirection / Aiming rotation modes,
	// and is used in the Cycle Blending Anim Layers to blend to the appropriate directional states.
	EMovementDirection CalculateMovementDirection() const;
	// Take the input angle and determine its quadrant (direction).
	// Use the current Movement Direction to increase or decrease the buffers on the angle ranges
	// for each quadrant.
	EMovementDirection CalculateQuadrant(EMovementDirection Current,
		                                 float FRThreshold, float FLThreshold,
		                                 float BRThreshold, float BLThreshold,
		                                 float Buffer, float Angle) const;
	bool AngleInRange(float Angle, float MinAngle, float MaxAngle, float Buffer, bool bIncreaseBuffer) const;

	// Only perform a Rotate In Place Check if the character is Aiming or in First Person.
	bool CanRotateInPlace() const;
	// Only perform a Turn In Place check if the character is looking toward the camera
	// in Third Person, and if the "Enable Transition" curve is fully weighted.
	// The Enable_Transition curve is modified within certain states of the AnimBP
	// so that the character can only turn while in those states.
	bool CanTurnInPlace() const;
	// Only perform a Dynamic Transition check if the "Enable Transition" curve is fully weighted.
	// The Enable_Transition curve is modified within certain states of the AnimBP
	// so that the character can only transition while in those states.
	bool CanDynamicTransition() const;

	void RotateInPlaceCheck();
	void TurnInPlaceCheck(float DeltaTime);
	// Check each foot to see if the location difference between the IK_Foot bone and
	// its desired / target location (determined via a virtual bone) exceeds a threshold.
	// If it does, play an additive transition animation on that foot.
	// The currently set transition plays the second half of a 2 foot transition animation,
	// so that only a single foot moves. Because only the IK_Foot bone can be locked,
	// the separate virtual bone allows the system to know its desired location when locked.
	void DynamicTransitionCheck();

	void TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool bOverrideCurrent);

	// Update IK helper functions
	// Foot Lock
	void SetFootLocking(FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
		                float& CurrentFootLockAlpha, FVector& CurrentFootLockLocation, FRotator& CurrentFootLockRotation, float DeltaSeconds);
	void SetFootLockOffsets(FVector& LocalLocation, FRotator& LocalRotation, float DeltaSeconds);
	// Offsets
	void SetFootOffsets(FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
					FVector& CurrentLocationTarget, FVector& CurrentLocationOffset, FRotator& CurrentRotationOffset, float DeltaSeconds);
	void SetPelvisIKOffset(FVector FootOffsetLTarget, FVector FootOffsetRTarget, float DeltaSeconds);

	// In Air
	void UpdateInAirValues();
	// Calculate the land prediction weight by tracing in the velocity direction to find a walkable surface the character is falling toward,
	// and getting the 'Time' (range of 0-1, 1 being maximum, 0 being about to land) till impact.
	// The Land Prediction Curve is used to control how the time affects the final weight for a smooth blend.
	float CalculateLandPrediction();
};
