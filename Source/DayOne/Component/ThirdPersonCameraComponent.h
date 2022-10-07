// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DayOne/Data/CameraModel.h"
#include "ThirdPersonCameraComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DAYONE_API UThirdPersonCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	static FName BoneNameRoot;
	static FName BoneNameHead;
	static FName SocketNameRightShoulder;
	static FName SocketNameLeftShoulder;
	
	friend class ABaseCharacter;
	UThirdPersonCameraComponent();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE FMinimalViewInfo GetViewInfo() const
	{
		FMinimalViewInfo OutResult;

		OutResult.Location = TargetCameraLocation;
		OutResult.Rotation = TargetCameraRotation;
		OutResult.FOV = TargetCameraFOV;

		return OutResult;
	}

private:
	// Pivot target always sync with Character.
	// This is the final target that the camera always try to catch.
	// Pivot location depend on socket/bone location of character skeleton in world space.
	// So we have to call this function to re-calculate the target transform
	// whenever the character has moved.
	// Maybe we need to draw a green sphere in pivot target location...
	FTransform GetPivotTarget() const;

	// Get the smoothed, lagged pivot target location
	FVector CalculateAxisIndependentLag(FVector CurrentLocation,
		                                FVector TargetLocation,
		                                FRotator CameraRotation,
		                                FVector LagSpeeds,
		                                float DeltaTime) const;

	// Get sphere ray trace parameters for camera collision test.
	// TraceOrigin depend on socket/bone location of character skeleton in world space.
	// So we have to call this function to re-calculate the target trace origin.
	void GetTraceParams(FVector& TraceOrigin, float& TraceRadius, ETraceTypeQuery& TraceChannel) const;

	// Load camera config table data.
	void LoadCameraModel();

	// Update camera config by character stance and gait state.
	// Must be called every tick
	void UpdateCameraSettings(float DeltaTime);
	
protected:
	UPROPERTY()
	class ABaseCharacter* Character;
	
	// Camera look to character from right shoulder or left?
	bool bRightShoulder;

	// Intermediate cached pivot target
	FTransform SmoothedPivotTarget;
	
	// The camera data table & config
	UPROPERTY(EditDefaultsOnly, Category="DataTable")
	UDataTable* CameraModel;
	FCameraData CameraData;
	FCameraSettings CurrentCameraSettings;

private:
	// The output results
	FVector TargetCameraLocation;
	FRotator TargetCameraRotation;
	float TargetCameraFOV;
};
