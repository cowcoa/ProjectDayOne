// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitResult)
{
	Super::Fire(HitResult);

	if (Projectile == nullptr) return;

	USkeletalMeshComponent* WeaponMesh = GetMesh();
	const USkeletalMeshSocket* MuzzleSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	FTransform MuzzleTransform = MuzzleSocket->GetSocketTransform(GetMesh());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	FVector HitDirection = HitResult - MuzzleTransform.GetLocation();
	GetWorld()->SpawnActor<AProjectile>(
		Projectile,
		MuzzleTransform.GetLocation(),
		HitDirection.Rotation(),
		SpawnParams
		);
}
