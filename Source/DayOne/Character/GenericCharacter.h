// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericCharacter.generated.h"

UCLASS()
class DAYONE_API AGenericCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGenericCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE void PickupTheGun(class AGun* Gun){ MyGun = Gun; }

protected:
	virtual void BeginPlay() override;

	// Process player input.
	void OnMoveForward(float Value);
	void OnMoveRight(float Value);
	void OnTurn(float Value);
	void OnLookUp(float Value);
	void OnEquipGun();
	void OnCrouchPressed();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraArm;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_MyGun)
	class AGun* MyGun;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION()
	void OnRep_MyGun(AGun* MyLastGun);

	UFUNCTION(Server, Reliable)
	void ServerEquipGun();

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combat; }

};
