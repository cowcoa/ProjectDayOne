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

	UPROPERTY(EditDefaultsOnly, Category=Camera)
	class USpringArmComponent* CameraArm;
	UPROPERTY(EditDefaultsOnly, Category=Camera)
	class UCameraComponent* Camera;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* CharacterName;
	
public:
	// Sets default values for this character's properties
	ASwatCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
