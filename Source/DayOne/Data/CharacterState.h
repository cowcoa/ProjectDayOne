#pragma once

UENUM(BlueprintType, meta=(ScriptName="MovementState"))
enum class EMovementState : uint8
{
	MS_None = 0 UMETA(DisplayName = "None"),
	MS_Grounded UMETA(DisplayName = "OnGround"),
	MS_InAir UMETA(DisplayName = "InAir"),
	MS_MAX
};

UENUM(BlueprintType, meta=(ScriptName="MovementAction"))
enum class EMovementAction : uint8
{
	MA_None = 0 UMETA(DisplayName = "None"),
	MA_Rolling UMETA(DisplayName = "Rolling"),
	MA_GettingUp UMETA(DisplayName = "GettingUp"),
	MA_MAX
};

UENUM(BlueprintType, meta=(ScriptName="GaitState"))
enum class EGaitState : uint8
{
	GS_Walking = 0 UMETA(DisplayName = "Walking"),
	GS_Running UMETA(DisplayName = "Running"),
	GS_Sprinting UMETA(DisplayName = "Sprinting"),
	GS_MAX
};

UENUM(BlueprintType, meta=(ScriptName="StanceState"))
enum class EStanceState : uint8
{
	SS_Standing = 0 UMETA(DisplayName = "Standing"),
	SS_Crouching UMETA(DisplayName = "Crouching"),
	SS_MAX
};

UENUM(BlueprintType, meta=(ScriptName="RotationMode"))
enum class ERotationMode : uint8
{
	// Always move toward the input velocity direction
	RM_Velocity = 0 UMETA(DisplayName = "Velocity"),
	// Always move toward the looking direction
	RM_Looking UMETA(DisplayName = "Looking"),
	RM_Aiming UMETA(DisplayName = "Aiming"),
	RM_MAX
};

UENUM(BlueprintType, meta=(ScriptName="MovementModel"))
enum class EMovementModel : uint8
{
	// Currently character only moving in normal model,
	// If we have movement related BUFF/DEBUFF, we then back to extend this.
	MM_Normal = 0 UMETA(DisplayName = "Normal"),
	MM_MAX
};

UENUM(BlueprintType, meta=(ScriptName="ViewMode"))
enum class EViewMode : uint8
{
	// Current character on in third person view,
	// We are considering make character in first person view while they are aiming.
	VM_ThirdPerson = 0 UMETA(DisplayName = "ThirdPerson"),
	VM_FirstPerson UMETA(DisplayName = "FirstPerson"),
	VM_MAX
};
