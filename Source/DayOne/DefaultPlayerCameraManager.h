// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "DefaultPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ADefaultPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
public:
	virtual void OnPossess(APawn* InPawn);

private:
	class APawn* ControlledPawn;
};
