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

public:	
	UCombatComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable)
	void EquipWeapon(ASwatCharacter* Character, AWeapon* Weapon);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AWeapon* GetWeapon() { return CurrentWeapon; }

private:
	UPROPERTY(Replicated)
	AWeapon* CurrentWeapon = nullptr;
	ASwatCharacter* AttachedCharacter;
};
