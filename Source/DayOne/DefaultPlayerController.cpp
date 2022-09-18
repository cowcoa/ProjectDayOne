// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"

#include "DefaultPlayerCameraManager.h"
#include "Character/BaseCharacter.h"

ADefaultPlayerController::ADefaultPlayerController()
	: DefaultCharacter(nullptr)
{
	if (GetCharacter() != nullptr)
	{
		DefaultCharacter = Cast<ABaseCharacter>(GetCharacter());
	}
}

void ADefaultPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ADefaultPlayerCameraManager* CameraManager = Cast<ADefaultPlayerCameraManager>(PlayerCameraManager);
	if (CameraManager)
	{
		CameraManager->OnPossess(InPawn);
	}
}
