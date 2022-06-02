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

	void ShowHeadDisplay(bool bShowHUD);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category="Gun")
	class USkeletalMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere, Category="Gun")
	class USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category="Display")
	class UWidgetComponent* HeadDisplay;

	
	UFUNCTION()
	virtual void OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 bOtherBodyIndex);

public:	
	virtual void Tick(float DeltaTime) override;

	void SetGunState(EGunState State);
private:
	UPROPERTY(ReplicatedUsing=OnRep_GunState,VisibleInstanceOnly)
	EGunState GunState;

	UFUNCTION()
	void OnRep_GunState(EGunState LastGunState);
};
