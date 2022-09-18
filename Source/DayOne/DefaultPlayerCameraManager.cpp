// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerCameraManager.h"

void ADefaultPlayerCameraManager::OnPossess(APawn* InPawn)
{
	ControlledPawn = InPawn;

	const AActor* MyViewTarget = GetViewTarget();
	UE_LOG(LogTemp, Warning, TEXT("MyViewTarget: %s"), *(MyViewTarget->GetName()))
}
