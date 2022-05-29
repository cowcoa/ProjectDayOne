// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gun.generated.h"

UENUM(BlueprintType)
enum class EGunState : uint8
{
	EGS_Init UMETA(DisplayName = "Init State"),
	EGS_Equipped UMETA(DisplayName = "Equipped State"),
	EGS_Dropped UMETA(DisplayName = "Dropped State"),
	EGS_Max UMETA(DisplayName = "Max Enum")
};

UCLASS()
class DAYONE_API AGun : public AActor
{
	GENERATED_BODY()
	
public:	
	AGun();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Gun")
	class USkeletalMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere, Category="Gun")
	class USphereComponent* CollisionSphere;

public:	
	virtual void Tick(float DeltaTime) override;
private:
	UPROPERTY(VisibleAnywhere)
	EGunState GunState;
};
