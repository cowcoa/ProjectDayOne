// Fill out your copyright notice in the Description page of Project Settings.


#include "SwatAnimInstance.h"

#include "SwatCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Navigation/PathFollowingComponent.h"

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
	bIsWeaponEquipped = Character->IsWeaponEquipped();

	// Get character crouch state
	bIsCrouched = Character->IsCrouching();

	// Get character aiming state
	bIsAiming = Character->IsAiming();

	// Get jog and lean info
	FRotator AimingDirRotation = Character->GetBaseAimRotation();
	FRotator MovingDirRotation = UKismetMathLibrary::MakeRotFromX(Character->GetVelocity());
	FRotator DeltaMovingRotation = UKismetMathLibrary::NormalizedDeltaRotator(MovingDirRotation, AimingDirRotation);
	CurrentDeltaMovingRotation = UKismetMathLibrary::RInterpTo(CurrentDeltaMovingRotation, DeltaMovingRotation, DeltaSeconds, 5);
	YawOffset = CurrentDeltaMovingRotation.Yaw;
	//UE_LOG(LogTemp, Warning, TEXT("Facing Rot: %f, Moving Rot: %f, Delta Rot: %f, YawOffset: %f"), AimingDirRotation.Yaw, MovingDirRotation.Yaw, DeltaMovingRotation.Yaw, YawOffset);
	
	LastFrameCharacterRotation = CurrFrameCharacterRotation;
	CurrFrameCharacterRotation = Character->GetActorRotation();
	//UE_LOG(LogTemp, Warning, TEXT("LastFrameCharacterRotation yaw: %f, CurrFrameCharacterRotation yaw: %f"), LastFrameCharacterRotation.Yaw, CurrFrameCharacterRotation.Yaw);
	FRotator DeltaActorRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrFrameCharacterRotation, LastFrameCharacterRotation);
	//UE_LOG(LogTemp, Warning, TEXT("LastFrameCharacterRotation yaw: %f, CurrentDeltaMovingRotation yaw: %f, DeltaActorRotation yaw: %f"), LastFrameCharacterRotation.Yaw, CurrentDeltaMovingRotation.Yaw, DeltaActorRotation.Yaw);
	
	float DeltaYaw = DeltaActorRotation.Yaw / DeltaSeconds;
	float CurrentDeltaYaw = UKismetMathLibrary::FInterpTo(Lean, DeltaYaw, DeltaSeconds, 5);
	Lean = UKismetMathLibrary::Clamp(CurrentDeltaYaw, -180, 180);
	//UE_LOG(LogTemp, Warning, TEXT("Lean: %f"), Lean);

	// Aim Offsets
	AOYaw = Character->GetAOYaw();
	AOPitch = Character->GetAOPitch();

	//UE_LOG(LogTemp, Warning, TEXT("AO Yaw: %f"), AOYaw);
}
