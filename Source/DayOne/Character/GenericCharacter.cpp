// Fill out your copyright notice in the Description page of Project Settings.


#include "GenericCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "DayOne/Components/CombatComponent.h"
#include "DayOne/Gun/Gun.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGenericCharacter::AGenericCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraArm"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->TargetArmLength = 800.f;
	CameraArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);

	// Init crouch movement.
    ACharacter::GetMovementComponent()->NavAgentProps.bCanCrouch = true;

#if UE_EDITOR
	GetArrowComponent()->SetHiddenInGame(false);
#endif 
}

void AGenericCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AGenericCharacter, MyGun, COND_OwnerOnly);
}

// Called when the game starts or when spawned
void AGenericCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AGenericCharacter::OnRep_MyGun(AGun* MyLastGun)
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_MyGun on client"));
	if (MyGun == nullptr)
	{
		MyLastGun->ShowHeadDisplay(false);
	} else
	{
		MyGun->ShowHeadDisplay(true);
	}
}

void AGenericCharacter::ServerEquipGun_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("----- ServerEquipGun_Implementation called -----"));

	if (MyGun && Combat)
	{
		Combat->EquipGun(MyGun);
	}
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
		PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Released, this, &ThisClass::StopJumping);
		PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::OnMoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::OnMoveRight);
		PlayerInputComponent->BindAxis("Turn", this, &ThisClass::OnTurn);
		PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::OnLookUp);

		PlayerInputComponent->BindAction("Equip", EInputEvent::IE_Pressed, this, &ThisClass::OnEquipGun);
		PlayerInputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ThisClass::OnCrouchPressed);

		PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Pressed, this, &ThisClass::OnAimPressed);
		PlayerInputComponent->BindAction("Aim", EInputEvent::IE_Released, this, &ThisClass::OnAimReleased);
	}
}

bool AGenericCharacter::IsAiming() const
{
	if (Combat && Combat->bAiming)
	{
		return true;
	}
	return false;
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

void AGenericCharacter::OnEquipGun()
{
	//UE_LOG(LogTemp, Warning, TEXT("OnEquipGun pressed!"));
	// Execute equip weapon on server.
	ServerEquipGun();
}

void AGenericCharacter::OnCrouchPressed()
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

void AGenericCharacter::OnAimPressed()
{
	if (Combat)
	{
		Combat->AimTarget(true);
	}
}

void AGenericCharacter::OnAimReleased()
{
	if (Combat)
	{
		Combat->AimTarget(false);
	}
}
