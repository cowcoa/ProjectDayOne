// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericAnimInstance.h"
#include "GenericCharacter.h"
#include "DayOne/Components/OldCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

	bIsAiming = Character->IsAiming();
	
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

	FRotator AimRtt =  Character->GetBaseAimRotation();
	FRotator MoveRtt =  UKismetMathLibrary::MakeRotFromX(Character->GetVelocity());
	FRotator DeltaRtt = UKismetMathLibrary::NormalizedDeltaRotator(MoveRtt,AimRtt);
	MoveOffsetRotation = FMath::RInterpTo(MoveOffsetRotation, DeltaRtt, DeltaSeconds, 5);
	YawOffset = MoveOffsetRotation.Yaw;
	//UE_LOG(LogTemp, Warning, TEXT("GetBaseAimRotation yaw: %f, GetVelocity yaw: %f, YawOffset: %f"), AimRtt.Yaw, MoveRtt.Yaw, YawOffset);
	 
	LastRotation = CurrentRotation;
	CurrentRotation = Character->GetActorRotation();
	FRotator CharDeltaRtt = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, LastRotation);
	UE_LOG(LogTemp, Warning, TEXT("LastActorRotation yaw: %f, CurrentActorRotation yaw: %f, DeltaActorRotation yaw: %f"), LastRotation.Yaw, CurrentRotation.Yaw, CharDeltaRtt.Yaw);

	float Target = CharDeltaRtt.Yaw / DeltaSeconds;
	float Interp = UKismetMathLibrary::FInterpTo(Lean, Target, DeltaSeconds, 6);
	Lean = UKismetMathLibrary::Clamp(Interp, -90, 90);
	UE_LOG(LogTemp, Warning, TEXT("Target: %f, Interp: %f, Lean: %f"), Target, Interp, Lean);
}
