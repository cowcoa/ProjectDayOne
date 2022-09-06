// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "DayOne/Character/SwatCharacter.h"
#include "DayOne/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// NOTE: This functions only execute on SERVER side
void UCombatComponent::EquipWeapon(ASwatCharacter* Character, AWeapon* Weapon)
{
	UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::EquipWeapon"));
	ASwatCharacter* OwnerCharacter = Cast<ASwatCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		const USkeletalMeshSocket* WeaponSocket = OwnerCharacter->GetMesh()->GetSocketByName(FName(TEXT("WeaponSocket")));
		WeaponSocket->AttachActor(Weapon, OwnerCharacter->GetMesh());
		Weapon->SetOwner(OwnerCharacter);

		UE_LOG(LogTemp, Warning, TEXT("set weapon state"));
		CurrentWeapon = Weapon;
		CurrentWeapon->SetState(EWeaponState::EWS_Equipped);

		UE_LOG(LogTemp, Warning, TEXT("AttachedCharacter"));
		OwnerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		OwnerCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::AimTarget(bool bAim)
{
	bIsAiming = bAim;
	ASwatCharacter* OwnerCharacter = Cast<ASwatCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	ServerAimTarget(bAim);
}

void UCombatComponent::ServerAimTarget_Implementation(bool bAim)
{
	bIsAiming = bAim;
	ASwatCharacter* OwnerCharacter = Cast<ASwatCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::Fire(bool bPressed)
{
	bFiring = bPressed;
	if (bFiring)
	{
		ServerFire();
	}
}

void UCombatComponent::ServerFire_Implementation()
{
	MulticastFire();
}

void UCombatComponent::MulticastFire_Implementation()
{
	ASwatCharacter* OwnerCharacter = Cast<ASwatCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		OwnerCharacter->PlayFireMontage(IsAiming());
		if (CurrentWeapon)
		{
			CurrentWeapon->Fire(HitTarget);
		}
	}
}

void UCombatComponent::TraceByCrosshair(FHitResult& HitResult)
{
	FVector2d ViewPortSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}
	
	FVector2d CrosshairPosition(ViewPortSize.X / 2.0f, ViewPortSize.Y / 2.0f);

	FVector WorldPosition;
	FVector WorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairPosition,
		WorldPosition,
		WorldDirection
		);

	if (bScreenToWorld)
	{
		FVector Start = WorldPosition;
		FVector End = Start + WorldDirection * 100000;
		GetWorld()->LineTraceSingleByChannel(HitResult,
			Start,
			End,
			ECC_Visibility
			);

		if (!HitResult.bBlockingHit)
		{
			HitResult.ImpactPoint = End;
			HitTarget = End;
		}
		else
		{
			//UE_LOG(LogTemp, Warning, TEXT("ImpactPoint:(%f,%f)"), HitResult.ImpactPoint.X, HitResult.ImpactPoint.Y);
			DrawDebugSphere(GetWorld(),
				HitResult.ImpactPoint,
				10.0f,
				10.0f,
				FColor::Red
				);
			HitTarget = HitResult.ImpactPoint;
		}
	}
}

void UCombatComponent::OnRep_CurrentWeapon()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon"));
	ASwatCharacter* Character = Cast<ASwatCharacter>(GetOwner());
	if (Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon"));
		// Change character moving state.
		//Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		//Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FHitResult HitResult;
	TraceByCrosshair(HitResult);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentWeapon);
	DOREPLIFETIME(ThisClass, bIsAiming);
}

