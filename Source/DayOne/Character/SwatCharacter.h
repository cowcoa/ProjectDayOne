// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DayOne/Component/CombatComponent.h"
#include "GameFramework/Character.h"
#include "SwatCharacter.generated.h"

UCLASS()
class DAYONE_API ASwatCharacter : public ACharacter
{
	GENERATED_BODY()
	friend class AWeapon;

public:
	ASwatCharacter();

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE bool IsWeaponEquipped() const { return Combat && Combat->GetWeapon() != nullptr; }
	FORCEINLINE bool IsCrouching() const { return bIsCrouched; }
	FORCEINLINE bool IsAiming() const { return Combat && Combat->IsAiming(); }

	FORCEINLINE float GetAOYaw() const { return AoYaw; }
	FORCEINLINE float GetAOPitch() const { return AoPitch; }

protected:
	// Player components
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	class UWidgetComponent* Hud;
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	class UCombatComponent* Combat;

	void UpdateAimOffset(float DeltaTime);
	
private:
	// Player input callback
	// Press W/S to move forward or backward
	void OnMoveForward(float Value);
	// Press A/D to move left or right
	void OnMoveRight(float Value);
	// Move your MOUSE to look around
	void OnTurn(float Value);
	void OnLookUp(float Value);
	// Press E to equip weapon
	void OnEquip();
	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon();
	// Press LEFT SHIFT to crouch
	void OnCrouch();
	// Press/Release RIGHT MOUSE BUTTON to aim/un-aim
	void OnAimHold();
	void OnAimRelease();
	
	// Weapon available near the character(can be picked up)
	UPROPERTY(ReplicatedUsing=OnRep_AvailableWeapon)
	class AWeapon* AvailableWeapon = nullptr;
	// @param LastWeapon - last value of AvailableWeapon
	UFUNCTION()
	void OnRep_AvailableWeapon(class AWeapon* LastWeapon);

	// AimOffsets
	float AoYaw = 0.0f;
	float AoPitch = 0.0f;
	FRotator StandAimDir;
};
