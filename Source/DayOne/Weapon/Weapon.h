// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Init UMETA(DisplayName = "Init State"),
	EWS_Equipped UMETA(DisplayName = "Equipped State"),
	EWS_Dropped UMETA(DisplayName = "Dropped State"),
	
	EWS_MAX
};

UCLASS()
class DAYONE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Show or hide the weapon's HUD
	UFUNCTION(BlueprintCallable)
	void SetHudVisibility(bool bNewVisibility);

	// Change weapon state;
	UFUNCTION(BlueprintCallable)
	void SetState(EWeaponState State);

	FORCEINLINE USkeletalMeshComponent* GetMesh() { return Mesh; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	virtual void OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComponent,
		                        AActor* OtherActor,
		                        UPrimitiveComponent* OtherComp,
		                        int32 OtherBodyIndex,
		                        bool bFromSweep,
		                        const FHitResult & SweepResult);
	UFUNCTION()
	virtual void OnColliderEndOverlap(UPrimitiveComponent* OverlappedComponent,
		                      AActor* OtherActor,
		                      UPrimitiveComponent* OtherComp,
		                      int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* Hud;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Collider;
	
	UPROPERTY(ReplicatedUsing=OnRep_CurrentState)
	EWeaponState CurrentState = EWeaponState::EWS_Init;
	UFUNCTION()
	void OnRep_CurrentState();
};
