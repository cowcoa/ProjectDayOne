// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericAnimInstance.h"
#include "GenericCharacter.h"
#include "DayOne/Components/CombatComponent.h"
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
	bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bIsGunEquipped = Character->GetCombatComponent()->GetGun() != nullptr;

	bIsCrouched = Character->GetMovementComponent()->IsCrouching();
	
	/*
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("Character speed: %f, falling: %s, accelerating: %s"), Speed, (bIsInAir ? TEXT("true") : TEXT("false")), (bIsAccelerating ? TEXT("true") : TEXT("false")))
		);
	}
	UE_LOG(LogTemp, Warning, TEXT("Character speed: %f, falling: %s, accelerating: %s"), Speed, (bIsInAir ? TEXT("true") : TEXT("false")), (bIsAccelerating ? TEXT("true") : TEXT("false")));
	*/
}
