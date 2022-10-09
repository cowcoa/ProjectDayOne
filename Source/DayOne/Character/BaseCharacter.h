// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DayOne/Component/LocomotionComponent.h"
#include "DayOne/Component/ThirdPersonCameraComponent.h"
#include "DayOne/Data/CharacterState.h"
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

public:
	ABaseCharacter(const FObjectInitializer& ObjectInitializer);

	FORCEINLINE ULocomotionComponent* GetLocomotionComponent() const
	{
		return Locomotion;
	}
	FORCEINLINE UThirdPersonCameraComponent* GetCameraComponent() const
	{
		return ThirdPersonCamera;
	}

	EGaitState GetGait() const;
	EStanceState GetStance() const;

protected:
	//
	// Inherited and override functions.
	//
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;
	virtual void Tick(float DeltaTime) override;

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
	// Actually move the character.
	void ProcessPlayerMovementInput(float bIsForwardAxis);
	// Get current player controller's forward and right direction vector
	void GetControlVector(OUT FVector& Forward, OUT FVector& Right) const;
	// Calculate the actual physical acceleration of the character.
	//FVector CalculateAcceleration() const;
	// Fix gamepad stick moving along diagonal problem.
	// Ref: https://forums.unrealengine.com/t/problem-with-rolling-template-diagonal-directions-give-almost-twice-the-power/391172
	void FixDiagonalGamepadValues(float InY, float InX, OUT float& OutY, OUT float& OutX);

private:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UThirdPersonCameraComponent* ThirdPersonCamera;

	// Properties
	UPROPERTY(EditAnywhere, Category="InputProperty", meta=(AllowPrivateAccess="true"))
	float LookupRate;
	UPROPERTY(EditAnywhere, Category="InputProperty", meta=(AllowPrivateAccess="true"))
	float TurnRate;

	// References
	UPROPERTY()
	class ULocomotionComponent* Locomotion;
};
