// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SwatCharacter.generated.h"

UCLASS()
class DAYONE_API ASwatCharacter : public ACharacter
{
	GENERATED_BODY()

	// Process player input.
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnTurn(float Value);
	void OnLookUp(float Value);
	void OnEquip();

	UPROPERTY(EditDefaultsOnly, Category=Camera)
	class USpringArmComponent* CameraArm;
	UPROPERTY(EditDefaultsOnly, Category=Camera)
	class UCameraComponent* Camera;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* CharacterName;
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;
	
public:
	// Sets default values for this character's properties
	ASwatCharacter();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Run-in-server RPC
	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE void EquipWeapon(class AWeapon* Weapon)
	{
		AvailableWeapon = Weapon;
	}
	FORCEINLINE void DropWeapon(){}

private:
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	class AWeapon* AvailableWeapon = nullptr;
	UFUNCTION()
	void OnRep_EquippedWeapon(class AWeapon* LastWeapon);
};
