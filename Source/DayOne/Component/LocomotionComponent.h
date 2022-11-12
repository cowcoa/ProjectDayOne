// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DayOne/Data/CharacterState.h"
#include "DayOne/Data/MovementModel.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LocomotionComponent.generated.h"


// UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
UCLASS()
class DAYONE_API ULocomotionComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:	
	friend class ABaseCharacter;
	ULocomotionComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	virtual void Crouch(bool bClientSimulation = false) override;
	virtual void UnCrouch(bool bClientSimulation = false) override;
	
protected:
	// Reference variables.
	UPROPERTY()
	class ABaseCharacter* Character;
	UPROPERTY()
	class UAnimInstance* MainAnimInstance;

	// Get movement data from the Movement Model Data table and set the Movement Data Struct.
	// This allows you to easily switch out movement behaviors.
	void SetMovementModel();
	// change gait, maybe remove later.
	void SetGait(EGaitState NewActualGait);
	// change movement state, air or grounded.
	void SetMovementState(EMovementState NewMovementState);
	// change stance, standing or crouching
	void SetStance(EStanceState NewStance);

	// Setup initial states and variables
	void SetEssentialValues();
	// Calculate the actual physical acceleration of the character.
	FVector CalculatePhysicalAcceleration() const;

	//
	void UpdateCharacterMovement();
	// Calculate the Allowed Gait.
	// This represents the maximum Gait the character is currently allowed to be in,
	// and can be determined by the desired gait, the rotation mode, the stance, etc.
	// For example, if you wanted to force the character into a walking state while indoors, this could be done here.
	EGaitState GetAllowedGait() const;
	// Determine if the character is currently able to sprint based on the Rotation mode and current acceleration (input) rotation.
	// If the character is in the Looking Rotation mode, only allow sprinting if there is full movement input and it is faced forward relative to the camera + or - 50 degrees.
	bool CanSprint() const;
	// Get the Actual Gait.
	// This is calculated by the actual movement of the character,
	// and so it can be different from the desired gait or allowed gait.
	// For instance, if the Allowed Gait becomes walking, the Actual gait will still be running untill the character decelerates to the walking speed.
	EGaitState GetActualGait(EGaitState AllowedGait) const;
	// Use the allowed gait to update the movement settings.
	void UpdateDynamicMovementSettings(EGaitState AllowedGait);
	// Get the Current Movement Settings.
	FMovementSettings GetTargetMovementSettings() const;
	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed, and 3 = the Sprint Speed.
	// This allows you to vary the movement speeds but still use the mapped range in calculations for consistent results.
	float GetMappedSpeed() const;

	// Must pay more attention to this method, this may auto rotate character
	void UpdateGroudedRotation();
	// Rotate character while moving??
	bool CanUpdateMovementRotation() const;
	// Interpolate the Target Rotation for extra smooth rotation behavior
	void SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed);
	// get curve value from anim instance
	float GetAnimCurveValue(FName CurveName) const;
	// Calculate the rotation rate by using the current Rotation Rate Curve in the Movement Settings.
	// Using the curve in conjunction with the mapped speed gives you a high level of control over the rotation rates for each speed.
	// Increase the speed if the camera is rotating quickly for more responsive rotation.
	float CalculateGroundedRotationRate() const;
	// Prevent the character from rotating past a certain angle.
	void LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed);
	
	// Cache certain values to be used in calculations on the next frame
	void CacheValues();
	
	// MovementSettings read from foreign table.
	// Currently we only support Normal movement state.
	// Possible future movement state including DE-BUFF/BUFF
	UPROPERTY(EditDefaultsOnly, Category="DataTable")
	UDataTable* MovementModel;
	FMovementData MovementData;
	FMovementSettings CurrentMovementSettings;
	
private:
	// Cached variables
	// Physical speed last frame
	FVector PreviousVelocity;
	// controller yaw last frame
	float PreviousAimYaw;

	// Essential info
	// Character physical acceleration
	FVector PhysicalAcceleration;
	// Character speed
	float Speed;
	// Character moving state
	bool bIsMoving;
	// Velocity direction of x axis
	FRotator LastVelocityRotation;
	// Current player's input amount on movement
	float MovementInputAmount;
	// player input state
	bool bHasMovementInput;
	// player input direction of x axis
	FRotator LastMovementInputRotation;
	// controller's yaw speed, between current frame and last frame.
	float AimYawRate;

	// Player input state variables
	EGaitState DesiredGait;

	// Actual state variables
	// Grounded or InAir(Falling)
	EMovementState MovementState;
	EMovementState PrevMovementState;
	// Standing or Crouching
	EStanceState Stance;
	EStanceState DesiredStance;
	// Looking or Aiming
	ERotationMode RotationMode;
	// Walking, Running or Sprinting
	EGaitState Gait;
	// None, Rolling or GettingUp
	EMovementAction MovementAction;

	// Rotation system
	FRotator TargetRotation;
	FRotator InAirRotation;
};
