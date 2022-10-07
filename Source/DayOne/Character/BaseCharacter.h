// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DayOne/Data/CharacterState.h"
#include "DayOne/Data/MovementModel.h"
#include "DayOne/Data/CameraModel.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class DAYONE_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	static FName MoveForwardInputName;
	static FName MoveRightInputName;
	static FName LookupInputName;
	static FName TurnInputName;
	static FName StanceInputName;

	// Movement
	UPROPERTY(EditDefaultsOnly, Category="DataTable")
	UDataTable* MovementModel;

private:
	// Properties:
	UPROPERTY()
	class UAnimInstance* MainAnimInstance;
	UPROPERTY(EditAnywhere, Category="InputProperty", meta=(AllowPrivateAccess="true"))
	float LookupRate;
	UPROPERTY(EditAnywhere, Category="InputProperty", meta=(AllowPrivateAccess="true"))
	float TurnRate;
	// MovementSettings read from foreign table.
	// Currently we only support Normal movement state.
	// Possible future movement state including DE-BUFF/BUFF
	FMovementSettingsState* MovementData;
	FMovementSettings CurrentMovementSettings;

	// States:
	// Player input state. e.g, move forward, jump, input acceleration
	UPROPERTY(EditAnywhere, Category="InputProperty", meta=(AllowPrivateAccess="true"))
	EGaitState DesiredGait;
	UPROPERTY(EditAnywhere, Category="InputProperty", meta=(AllowPrivateAccess="true"))
	ERotationMode DesiredRotationMode;
	UPROPERTY(EditAnywhere, Category="InputProperty", meta=(AllowPrivateAccess="true"))
	EStanceState DesiredStance;
	
	// Character dynamic calculated state. e.g, current speed, physical acceleration
	// Physical acceleration
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FVector Acceleration;
	// Horizontal speed
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	float Speed;
	// if speed > 0
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	bool bIsMoving;
	// current velocity orientation (to x axis)
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FRotator LastVelocityRotation;
	// player input strength, from 0 to 1, current input acc / max input acc
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	float MovementInputAmount;
	// input amount > 0
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	bool bHasMovementInput;
	// player input orientation (to x axis)
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FRotator LastMovementInputRotation;
	// delta controller orientation.
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	float AimYawRate;

	// Character data cache state. e.g, previous speed
	// physical speed last frame
	FVector PreviousVelocity;
	// controller orientation last frame
	float PreviousAimYaw;

	// Character logic state. e.g, standing, crouching
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	EGaitState Gait;
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	EMovementState MovementState;
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	EMovementAction MovementAction;
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	EStanceState Stance;
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	ERotationMode RotationMode;

	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FRotator TargetRotation;
	
public:
	ABaseCharacter();

	FORCEINLINE EGaitState GetGait() const
	{
		return Gait;
	}

	FORCEINLINE EStanceState GetStance() const
	{
		return Stance;
	}

protected:
	//
	// Inherited and override functions.
	//
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	
	virtual void Tick(float DeltaTime) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	
	//
	// Input callback functions.
	//
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnLookUp(float Value);
	void OnTurn(float Value);

	// Change stance between Standing and Crouching
	void OnStance();
	
	//
	// Utility functions.
	//
	// Get current player controller's forward and right direction vector
	void GetControlVector(OUT FVector& Forward, OUT FVector& Right) const;
	// Actually move the character.
	void ProcessPlayerMovementInput(float bIsForwardAxis);
	// Calculate the actual physical acceleration of the character.
	FVector CalculateAcceleration() const;
	// Fix gamepad stick moving along diagonal problem.
	// Ref: https://forums.unrealengine.com/t/problem-with-rolling-template-diagonal-directions-give-almost-twice-the-power/391172
	void FixDiagonalGamepadValues(float InY, float InX, OUT float& OutY, OUT float& OutX);
	//
	void UpdateCharacterMovement();
	void UpdateGroudedRotation();

	// Calculate the Allowed Gait.
	// This represents the maximum Gait the character is currently allowed to be in,
	// and can be determined by the desired gait, the rotation mode, the stance, etc.
	// For example, if you wanted to force the character into a walking state while indoors, this could be done here.
	EGaitState GetAllowedGait() const;

	// Get the Actual Gait.
	// This is calculated by the actual movement of the character,
	// and so it can be different from the desired gait or allowed gait.
	// For instance, if the Allowed Gait becomes walking, the Actual gait will still be running untill the character decelerates to the walking speed.
	EGaitState GetActualGait(EGaitState AllowedGait) const;

	// Determine if the character is currently able to sprint based on the Rotation mode and current acceleration (input) rotation.
	// If the character is in the Looking Rotation mode, only allow sprinting if there is full movement input and it is faced forward relative to the camera + or - 50 degrees.
	bool CanSprint() const;

	// Use the allowed gait to update the movement settings.
	void UpdateDynamicMovementSettings(EGaitState AllowedGait);

	// Get the Current Movement Settings.
	FMovementSettings GetTargetMovementSettings() const;

	bool CanUpdateMovementRotation() const;

	// get curve value from anim instance
	float GetAnimCurveValue(FName CurveName) const;

	// Calculate the rotation rate by using the current Rotation Rate Curve in the Movement Settings.
	// Using the curve in conjunction with the mapped speed gives you a high level of control over the rotation rates for each speed.
	// Increase the speed if the camera is rotating quickly for more responsive rotation.
	float CalculateGroundedRotationRate();

	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed, and 3 = the Sprint Speed.
	// This allows you to vary the movement speeds but still use the mapped range in calculations for consistent results.
	float GetMappedSpeed();

	// Prevent the character from rotating past a certain angle.
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed);

	// Internal functions
private:
	void OnMovementStateChanged(EMovementState NewState);
	void OnGaitChanged(EGaitState NewActualGait);
	void OnRotationModeChanged(ERotationMode NewRotationMode);
	
	
	void SetEssentialValues();
	// Cache certain values to be used in calculations on the next frame
	void CacheValues();
	void SetMovementModel();

	void OnStanceChanged(EStanceState NewStance);
	


	
	

	

	
	

	

	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed);







	// camera behaviors
	void SetCameraModel();
	// Update camera parameters by character states
	FCameraSettings GetTargetCameraSettings();
	
private:
	// Cache variables


	//
	// Player input changed states
	//




protected:
	// Character current pose states


	// Character current movement states










	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FRotator InAirRotation;

	// Some convince reference objects


	// Camera patameters

	
	// Data tables

	// Camera
	UPROPERTY(EditDefaultsOnly, Category="DataTable")
	UDataTable* CameraModel;
	FCameraData CameraData;
	FCameraSettings CurrentCameraSettings;

private:
	// camera relative
	FTransform GetPivotTarget();
	FRotator TargetCameraRotation;
	FTransform SmoothedPivotTarget;
	FVector CalculateAxisIndependentLag(FVector CurrentLocation, FVector TargetLocation, FRotator CameraRotation, FVector LagSpeeds);
	FVector PivotLocation;
	FVector TargetCameraLocation;
	UPROPERTY(EditAnywhere)
	float ThirdPersonFOV;
	bool bRightShoulder;
	void GetTraceParams(FVector& TraceOrigin, float& TraceRadius, ETraceTypeQuery& TraceChannel);

	//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UThirdPersonCameraComponent* ThirdPersonCamera;
};
