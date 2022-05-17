// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AGenericCharacter::AGenericCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->TargetArmLength = 600.f;
	CameraArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void AGenericCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Process player input.
void AGenericCharacter::OnMoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AGenericCharacter::OnMoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AGenericCharacter::OnTurn(float Value)
{
	AddControllerYawInput(Value);
}

void AGenericCharacter::OnLookUp(float Value)
{
	AddControllerPitchInput(Value);
}

// Called every frame
void AGenericCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGenericCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ThisClass::Jump);
		PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::OnMoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::OnMoveRight);
		PlayerInputComponent->BindAxis("Turn", this, &ThisClass::OnTurn);
		PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::OnLookUp);
	}
}

