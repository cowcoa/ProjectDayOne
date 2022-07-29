// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "DayOne/Character/SwatCharacter.h"
#include "DayOne/Weapon/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// NOTE: This functions only execute on SERVER side
void UCombatComponent::EquipWeapon(ASwatCharacter* Character, AWeapon* Weapon)
{
	const USkeletalMeshSocket* WeaponSocket = Character->GetMesh()->GetSocketByName(FName(TEXT("WeaponSocket")));
	WeaponSocket->AttachActor(Weapon, Character->GetMesh());
	Weapon->SetOwner(Character);

	CurrentWeapon = Weapon;
	CurrentWeapon->SetState(EWeaponState::EWS_Equipped);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentWeapon);
}

