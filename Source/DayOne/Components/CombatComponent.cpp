// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "DayOne/Character/GenericCharacter.h"
#include "DayOne/Gun/Gun.h"
#include "Engine/SkeletalMeshSocket.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
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
	}
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

