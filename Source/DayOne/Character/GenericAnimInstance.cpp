// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericAnimInstance.h"
#include "GenericCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UGenericAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<AGenericCharacter>(TryGetPawnOwner());
}

void UGenericAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character == nullptr)
	{
		Character = Cast<AGenericCharacter>(TryGetPawnOwner());
	}

	if (Character == nullptr) return;

	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	BIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
