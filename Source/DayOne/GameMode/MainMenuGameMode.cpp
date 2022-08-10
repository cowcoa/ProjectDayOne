// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuGameMode.h"

void AMainMenuGameMode::StartPlay()
{
	Super::StartPlay();

	/*
	if (GetWorld()->GetFirstPlayerController() != nullptr)
	{
		FInputModeUIOnly UIOnlyMode;
		GetWorld()->GetFirstPlayerController()->SetInputMode(UIOnlyMode);
		GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);

		UE_LOG(LogTemp, Warning, TEXT("Player Controller is ready!"))
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Player Controller is not ready!"))
	}
	*/
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
	
}

