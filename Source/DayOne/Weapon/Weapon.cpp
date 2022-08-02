// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "DayOne/Character/SwatCharacter.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	// Disable actor tick
	PrimaryActorTick.bCanEverTick = false;
	// Enable network replication
	bReplicates = true;

	// Setup weapon mesh and it's corresponding collision
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(Mesh);

	// Setup collider and disable ti by default
	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	Collider->SetSphereRadius(50);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Collider->SetupAttachment(RootComponent);

	// Setup HUD text component
	Hud = CreateDefaultSubobject<UWidgetComponent>(TEXT("HeadUpDisplay"));
	Hud->SetupAttachment(RootComponent);
	SetHudVisibility(false);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, CurrentState, COND_OwnerOnly);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// Enable collider on server only
	if (HasAuthority())
	{
		Collider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		Collider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Collider->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnColliderBeginOverlap);
		Collider->OnComponentEndOverlap.AddDynamic(this, &ThisClass::AWeapon::OnColliderEndOverlap);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                                     AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp,
                                     int32 OtherBodyIndex,
                                     bool bFromSweep,
                                     const FHitResult& SweepResult)
{
	ASwatCharacter* SwatCharacter = Cast<ASwatCharacter>(OtherActor);
	if (SwatCharacter)
	{
		SwatCharacter->SetAvailableWeapon(this);
	}
}

void AWeapon::OnColliderEndOverlap(UPrimitiveComponent* OverlappedComponent,
	                               AActor* OtherActor,
	                               UPrimitiveComponent* OtherComp,
	                               int32 OtherBodyIndex)
{
	ASwatCharacter* SwatCharacter = Cast<ASwatCharacter>(OtherActor);
	if (SwatCharacter)
	{
		SwatCharacter->SetAvailableWeapon(nullptr);
	}
}

void AWeapon::SetHudVisibility(bool bNewVisibility)
{
	if (Hud)
	{
		Hud->SetVisibility(bNewVisibility);
	}
}

void AWeapon::SetState(EWeaponState State)
{
	CurrentState = State;
	switch (CurrentState)
	{
	case EWeaponState::EWS_Init:
		break;
	case EWeaponState::EWS_Equipped:
		Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	default:
		checkNoEntry();
		break;
	}
}

void AWeapon::OnRep_CurrentState()
{
	switch (CurrentState)
	{
	case EWeaponState::EWS_Init:
		break;
	case EWeaponState::EWS_Equipped:
		SetHudVisibility(false);
		break;
	case EWeaponState::EWS_Dropped:
		break;
	default:
		break;
	}
}
