// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DefaultPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ADefaultPlayerController();

	virtual void OnPossess(APawn* InPawn) override;

private:
	class ABaseCharacter* DefaultCharacter;
};
