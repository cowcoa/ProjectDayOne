// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "DayOne/Character/SwatCharacter.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	// Enable replicate
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

	UE_LOG(LogTemp, Warning, TEXT("Weapon Constructor"))
	ShowHud(false);
}

void AWeapon::ShowHud(bool bNewVisibility)
{
	if (Hud)
	{
		Hud->SetVisibility(bNewVisibility);
		UE_LOG(LogTemp, Warning, TEXT("=== ShowHeadDisplay: %s ==="), bNewVisibility ? TEXT("true") : TEXT("false"));
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Weapon BeginPlay and Has Authority"));
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
	UE_LOG(LogTemp, Warning, TEXT("=== OnCollisionBeginOverlap called! ==="));
	//UE_LOG(LogTemp, Warning, TEXT("OnColliderBeginOverlap with %s"), *(OtherActor->GetName()));
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
	UE_LOG(LogTemp, Warning, TEXT("OnColliderEndOverlap with %s"), *(OtherActor->GetName()));
	ASwatCharacter* SwatCharacter = Cast<ASwatCharacter>(OtherActor);
	if (SwatCharacter)
	{
		SwatCharacter->EquipWeapon(nullptr);
	}
}
