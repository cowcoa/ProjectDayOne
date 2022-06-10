// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "DayOne/Character/GenericCharacter.h"
#include "DayOne/Gun/Gun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MyGun);
	DOREPLIFETIME(ThisClass, bAiming);
}

void UCombatComponent::EquipGun(AGun* Gun)
{
	AGenericCharacter* Character = Cast<AGenericCharacter>(GetOwner());
	if (Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("EquipGun Gun: %s"), *Gun->GetName());
		
		const USkeletalMeshSocket* GunSocket = Character->GetMesh()->GetSocketByName("GunSocket");
		GunSocket->AttachActor(Gun, Character->GetMesh());
		Gun->SetOwner(Character);
		Gun->SetGunState(EGunState::EGS_Equipped);
		MyGun = Gun;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UCombatComponent::AimTarget(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerAimTarget(bIsAiming);
}

void UCombatComponent::ServerAimTarget_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UCombatComponent::OnRep_MyGun(AGun* MyLastGun)
{
	AGenericCharacter* Character = Cast<AGenericCharacter>(GetOwner());
	if (Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::OnRep_MyGun"));
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

