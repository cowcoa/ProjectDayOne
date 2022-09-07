// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletCasing.generated.h"

UCLASS()
class DAYONE_API ABulletCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletCasing();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent,
				   AActor* OtherActor,
				   UPrimitiveComponent* OtherComp,
				   FVector NormalImpulse,
				   const FHitResult& Hit);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
	float CasingImpulseForce;

	UPROPERTY(EditAnywhere)
	class USoundCue* HitGroudSound;
};
