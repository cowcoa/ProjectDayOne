// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class DAYONE_API AProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	
public:	
	AProjectile();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent,
		               AActor* OtherActor,
		               UPrimitiveComponent* OtherComp,
		               FVector NormalImpulse,
		               const FHitResult& Hit);

	virtual void Destroyed() override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	class UParticleSystemComponent* ParticleSystemComponent;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* HitEffect;
	UPROPERTY(EditAnywhere)
	class USoundCue* HitSound;
};
