// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ASwatCharacter;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DAYONE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class ASwatCharacter;

public:	
	UCombatComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(ASwatCharacter* Character, AWeapon* Weapon);

	UFUNCTION(BlueprintCallable)
	void AimTarget(bool bAim);
	UFUNCTION(Server, Reliable)
	void ServerAimTarget(bool bAim);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AWeapon* GetWeapon() { return CurrentWeapon; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsAiming() const { return bIsAiming; }

	void Fire(bool bPressed);
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& HitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& HitTarget);

	void TraceByCrosshair(FHitResult& HitResult);

private:
	UPROPERTY(ReplicatedUsing=OnRep_CurrentWeapon)
	AWeapon* CurrentWeapon = nullptr;
	UFUNCTION()
	void OnRep_CurrentWeapon();
	
	ASwatCharacter* AttachedCharacter = nullptr;

	UPROPERTY(Replicated, Transient)
	bool bIsAiming;

	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	float BaseWalkSpeed = 600;
	UPROPERTY(EditDefaultsOnly, meta=(AllowPrivateAccess="true"))
	float AimWalkSpeed = 300;

	bool bFiring;
};
