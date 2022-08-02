// Fill out your copyright notice in the Description page of Project Settings.


#include "SwatAnimInstance.h"

#include "SwatCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void USwatAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<ASwatCharacter>(TryGetPawnOwner());
}

void USwatAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Get character anyway.
	if (Character == nullptr)
	{
		Character = Cast<ASwatCharacter>(TryGetPawnOwner());
		if (Character == nullptr) return;
	}

	// Compute character speed on XY plane
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	// Get character jumping status
	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	// Accelerating indicates whether the player is still input 
	bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	// Get character weapon state
	bIsWeaponEquipped = Character->WeaponEquipped();

	// Get character crouch state
	bIsCrouched = Character->Crouched();

	// Get character aiming state
	bIsAiming = Character->IsAiming();
}
