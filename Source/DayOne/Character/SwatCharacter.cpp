// Fill out your copyright notice in the Description page of Project Settings.


#include "SwatCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "DayOne/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ASwatCharacter::ASwatCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Enable replicate
	bReplicates = true;

	// By default character never rotate itself with controller.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	// In some situation we need our character always facing the controller's yaw direction, then setting this to true.
	bUseControllerRotationYaw = false;

	// Make character automatically rotate itself to face the movement direction.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	// Create camera spring arm and attach camera
	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->TargetArmLength = 800.f;
	CameraArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	CharacterName = CreateDefaultSubobject<UWidgetComponent>(TEXT("CharacterName"));
	CharacterName->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ASwatCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASwatCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, EquippedWeapon, COND_OwnerOnly);
}

// Called every frame
void ASwatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASwatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ThisClass::Jump);
		PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ThisClass::StopJumping);
		PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::OnMoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::OnMoveRight);
		PlayerInputComponent->BindAxis("Turn", this, &ThisClass::OnTurn);
		PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::OnLookUp);
	}
}

void ASwatCharacter::OnRep_EquippedWeapon(AWeapon* LastWeapon)
{
	if (EquippedWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_EquippedWeapon EquippedWeapon"))
		EquippedWeapon->ShowHud(true);
	}
	else if (LastWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_EquippedWeapon LastWeapon"))
		LastWeapon->ShowHud(false);
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
