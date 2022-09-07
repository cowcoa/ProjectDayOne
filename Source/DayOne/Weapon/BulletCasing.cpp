// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletCasing.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ABulletCasing::ABulletCasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	// never block player's camera
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	// Enable OnHit event notification.
	CasingMesh->SetNotifyRigidBodyCollision(true);

	CasingImpulseForce = 10.0f;
}

void ABulletCasing::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Warning, TEXT("ABulletCasing::OnHit"));
	if (HitGroudSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Play HitGroudSound"));
		UGameplayStatics::PlaySoundAtLocation(this, HitGroudSound, GetActorLocation());
	}
	Destroy();
}

void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector() * CasingImpulseForce);
}
