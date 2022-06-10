// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DAYONE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	friend class AGenericCharacter;
	
public:	
	// Sets default values for this component's properties
	UCombatComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipGun(class AGun* Gun);
	FORCEINLINE AGun* GetGun() const { return MyGun; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void AimTarget(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerAimTarget(bool bIsAiming);

private:
	UPROPERTY(ReplicatedUsing=OnRep_MyGun)
	class AGun* MyGun;

	UFUNCTION()
	void OnRep_MyGun(AGun* MyLastGun);

	UPROPERTY(Replicated)
	bool bAiming = false;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
