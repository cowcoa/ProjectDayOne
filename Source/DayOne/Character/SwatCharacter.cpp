// Fill out your copyright notice in the Description page of Project Settings.


#include "SwatCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "DayOne/Component/CombatComponent.h"
#include "DayOne/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

ASwatCharacter::ASwatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Enable replication
	bReplicates = true;

	// By default character never rotate itself with controller
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	// When player strafing with gun, we need set this back to true
	bUseControllerRotationYaw = false;

	// Create camera spring arm and attach camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create HUD component
	Hud = CreateDefaultSubobject<UWidgetComponent>(TEXT("Hud"));
	Hud->SetupAttachment(RootComponent);

	// Create custom combat component
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	// Set capsule and mesh ignore collider with camera
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ASwatCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Make character automatically rotate itself to face the movement direction
	// When player strafing with gun, we need set this back to false 
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	// Enable crouch by default
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void ASwatCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASwatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAimOffset(DeltaTime);
}

// Called to bind functionality to input
void ASwatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ThisClass::Jump);
		PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ThisClass::StopJumping);
		PlayerInputComponent->BindAction("Equip", EInputEvent::IE_Pressed, this, &ThisClass::OnEquip);
		PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ThisClass::OnCrouch);
		PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &ThisClass::OnAimHold);
		PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &ThisClass::OnAimRelease);
		
		PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::OnMoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::OnMoveRight);
		PlayerInputComponent->BindAxis("Turn", this, &ThisClass::OnTurn);
		PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::OnLookUp);
	}
}

void ASwatCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, AvailableWeapon, COND_OwnerOnly);
}

void ASwatCharacter::UpdateAimOffset(float DeltaTime)
{
	if (Combat == nullptr || Combat->GetWeapon() == nullptr) return;

	// Compute character speed on XY plane
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	//UE_LOG(LogTemp, Warning, TEXT("Speed: %f"), Speed);
	
	// We are standing
	if (Speed == 0.0f && !bIsInAir)
	{
		
		FRotator CurrentAimDir = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator AimOffset = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimDir, StandAimDir);
		AoYaw = AimOffset.Yaw;

		if (HasAuthority() && !IsLocallyControlled())
		{
			UE_LOG(LogTemp, Warning, TEXT("GetBaseAimRotation().Yaw in Character: %f"), GetBaseAimRotation().Yaw);
			UE_LOG(LogTemp, Warning, TEXT("AO Yaw in Character: %f"), AoYaw);
		}
		bUseControllerRotationYaw = false;
	}
	// We are moving
	if (Speed > 0.0f || bIsInAir)
	{
		StandAimDir = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AoYaw = 0.0f;

		UE_LOG(LogTemp, Warning, TEXT("Moving"));
		bUseControllerRotationYaw = true;
	}

	AoPitch = GetBaseAimRotation().Pitch;
	if (AoPitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AoPitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AoPitch);
	}
}

// Implement player input callback
void ASwatCharacter::OnMoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		// Always move forward a character toward the facing direction.
		const FRotator ControllerYawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector FaceDirection(FRotationMatrix(ControllerYawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(FaceDirection, Value);
	}
}

void ASwatCharacter::OnMoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		// Always move right/left a character toward the 90 degrees of facing direction.
		const FRotator ControllerYawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector FaceDirection(FRotationMatrix(ControllerYawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(FaceDirection, Value);
	}
}

void ASwatCharacter::OnTurn(float Value)
{
	// Turn the controller's yaw rotation to rotate the character itself.
	AddControllerYawInput(Value);
}

void ASwatCharacter::OnLookUp(float Value)
{
	// Turn the controller's pitch rotation to make the character's camera look up and down.
	AddControllerPitchInput(Value);
}

void ASwatCharacter::OnEquip()
{
	if (AvailableWeapon)
	{
		// Change character moving state.
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
		ServerEquipWeapon();
	}
}

void ASwatCharacter::ServerEquipWeapon_Implementation()
{
	if (AvailableWeapon && Combat)
	{
		UE_LOG(LogTemp, Warning, TEXT("Combat->EquipWeapon"));
		Combat->EquipWeapon(this, AvailableWeapon);
	}
}

void ASwatCharacter::OnCrouch()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ASwatCharacter::OnAimHold()
{
	if (IsWeaponEquipped())
	{
		Combat->AimTarget(true);
	}
}

void ASwatCharacter::OnAimRelease()
{
	if (IsWeaponEquipped())
	{
		Combat->AimTarget(false);
	}
}

void ASwatCharacter::OnRep_AvailableWeapon(AWeapon* LastWeapon)
{
	if (AvailableWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_AvailableWeapon AvailableWeapon"))
		AvailableWeapon->SetHudVisibility(true);
	}
	else if (LastWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_AvailableWeapon LastWeapon"))
		LastWeapon->SetHudVisibility(false);
	}
}
