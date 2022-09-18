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
	ABaseCharacter();

protected:
	//
	// Inherited and override functions.
	//
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//
	// Input callback functions.
	//
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnLookUp(float Value);
	void OnTurn(float Value);
	
	//
	// Utility functions.
	//
	// Get current player controller's forward and right direction vector
	void GetControlVector(OUT FVector& Forward, OUT FVector& Right);
	// Actually move the character.
	void ProcessPlayerMovementInput(float bIsForwardAxis);
	// Calculate the actual physical acceleration of the character.
	FVector CalculateAcceleration() const;

	// Internal functions
private:
	void OnMovementStateChanged(EMovementState NewState);
	void OnGaitChanged(EGaitState NewState);
	void OnRotationModeChanged(ERotationMode NewMode);
	
	void FixDiagonalGamepadValues(float InY, float InX, OUT float& OutY, OUT float& OutX);
	void SetEssentialValues();
	void CacheValues();
	void SetMovementModel();
	// Calculate the Allowed Gait.
	// This represents the maximum Gait the character is currently allowed to be in,
	// and can be determined by the desired gait, the rotation mode, the stance, etc.
	// For example, if you wanted to force the character into a walking state while indoors, this could be done here.
	EGaitState GetAllowedGait();
	// Get the Actual Gait.
	// This is calculated by the actual movement of the character,
	// and so it can be different from the desired gait or allowed gait.
	// For instance, if the Allowed Gait becomes walking, the Actual gait will still be running untill the character decelerates to the walking speed.
	EGaitState GetActualGait(EGaitState AllowedGait);
	
	bool CanSprint();

	void UpdateCharacterMovement();
	void UpdateGroudedRotation();

	bool CanUpdateMovementRotation();

	FMovementSettings GetTargetMovementSettings();
	void UpdateDynamicMovementSettings(EGaitState AllowedGait);

	float GetAnimCurveValue(FName CurveName);

	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed);

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

	// camera behaviors
	void SetCameraModel();
	// Update camera parameters by character states
	void GetTargetCameraSettings();
	
	
private:
	// Cache variables
	FVector PreviousVelocity;
	float PreviousAimYaw;

	//
	// Player input changed states
	//
	UPROPERTY(EditAnywhere, Category="InputProperty")
	EGaitState DesiredGait;
	UPROPERTY(EditAnywhere, Category="InputProperty")
	ERotationMode DesiredRotationMode;
	UPROPERTY(EditAnywhere, Category="InputProperty")
	EStanceState DesiredStance;
	UPROPERTY(EditAnywhere, Category="InputProperty")
	float LookupRate;
	UPROPERTY(EditAnywhere, Category="InputProperty")
	float TurnRate;

protected:
	// Character current pose states
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	EMovementState MovementState;
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	EMovementAction MovementAction;
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	EGaitState Gait;
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	EStanceState Stance;
	UPROPERTY(VisibleAnywhere, Category="CharacterState")
	ERotationMode RotationMode;

	// Character current movement states
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FVector Acceleration;
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	float Speed;
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	bool bIsMoving;
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FRotator TargetRotation;
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FRotator LastVelocityRotation;
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	float MovementInputAmount;
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	bool bHasMovementInput;
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FRotator LastMovementInputRotation;
	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	float AimYawRate;

	UPROPERTY(VisibleAnywhere, Category="CharacterProperty")
	FRotator InAirRotation;

	// Some convince reference objects
	UPROPERTY()
	class UAnimInstance* MainAnimInstance;

	// Data tables
	// Movement
	UPROPERTY(EditDefaultsOnly, Category="DataTable")
	UDataTable* MovementModel;
	FMovementSettingsState* MovementData;
	FMovementSettings CurrentMovementSettings;
	// Camera
	UPROPERTY(EditDefaultsOnly, Category="DataTable")
	UDataTable* CameraModel;
	FCameraData CameraData;
	FCameraSettings CurrentCameraSettings;
};
