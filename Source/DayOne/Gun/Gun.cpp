// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "DayOne/Character/GenericCharacter.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	// Setup sphere collision component.
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// We only enable this on server.
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SetRootComponent(CollisionSphere);

	// Setup skeletal mesh component.
	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMesh"));
	// Need to collide with ground and wall.
	GunMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	// But should let the player passthrough.
	GunMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->SetupAttachment(RootComponent);

	HeadDisplay = CreateDefaultSubobject<UWidgetComponent>(TEXT("HeadDisplay"));
	HeadDisplay->SetupAttachment(RootComponent);
}

void AGun::ShowHeadDisplay(bool bShowHUD)
{
	if (HeadDisplay)
	{
		HeadDisplay->SetVisibility(bShowHUD);
		UE_LOG(LogTemp, Warning, TEXT("=== ShowHeadDisplay: %s ==="), bShowHUD ? TEXT("true") : TEXT("false"));
	}
}

void AGun::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, GunState, COND_OwnerOnly);
}

// Called when the game starts or when spawned
void AGun::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		// Player can overlap with this collision to trigger some events.
		CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnCollisionBeginOverlap);
		CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnCollisionEndOverlap);

		GunState = EGunState::EGS_Init;

		UE_LOG(LogTemp, Warning, TEXT("=== OnCollisionBeginOverlap registered! ==="));

	}

			
	if (HeadDisplay)
	{
		HeadDisplay->SetVisibility(false);
	}
}

// Called every frame
void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGun::SetGunState(EGunState State)
{
	GunState = State;
	switch (GunState)
	{
	case EGunState::EGS_Equipped:
		CollisionSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("=== GunState is in unknown state ==="));
	}
}

void AGun::OnRep_GunState(EGunState LastGunState)
{
	ShowHeadDisplay(false);
}

void AGun::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("=== OnCollisionBeginOverlap called! ==="));
	
	AGenericCharacter* Character = Cast<AGenericCharacter>(OtherActor);
	if (Character)
	{
		Character->PickupTheGun(this);
	}
}

void AGun::OnCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 bOtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("=== OnCollisionEndOverlap called! ==="));
	
	AGenericCharacter* Character = Cast<AGenericCharacter>(OtherActor);
	if (Character)
	{
		Character->PickupTheGun(nullptr);
	}
}

