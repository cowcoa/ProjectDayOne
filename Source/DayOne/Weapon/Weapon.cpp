// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "DayOne/Character/SwatCharacter.h"

AWeapon::AWeapon()
{
	// We don't need this actor to tick every frame right now
	PrimaryActorTick.bCanEverTick = false;
	// Enable network replication
	bReplicates = true;

	// Setup weapon mesh and it's corresponding collision
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(Mesh);

	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	Collider->SetSphereRadius(50);
	Collider->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Collider->SetupAttachment(RootComponent);

	Hud = CreateDefaultSubobject<UWidgetComponent>(TEXT("HeadUpDisplay"));
	Hud->SetupAttachment(RootComponent);
	SetHudVisibility(false);
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
		SwatCharacter->EquipWeapon(this);
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
		SwatCharacter->EquipWeapon(nullptr);
	}
}

void AWeapon::SetHudVisibility(bool bNewVisibility)
{
	if (Hud)
	{
		Hud->SetVisibility(bNewVisibility);
	}
}