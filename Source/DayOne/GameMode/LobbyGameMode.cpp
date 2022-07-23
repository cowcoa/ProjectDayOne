// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	int32 NumOfPlayers = GameState.Get()->PlayerArray.Num();
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("Current Player Number: %d"), NumOfPlayers)
		);
	}

	if (NumOfPlayers == 2)
	{
		bUseSeamlessTravel = true;
		UWorld* World = GetWorld();
		World->ServerTravel("/Game/Maps/SwatTmp/Battle?listen");
	}
}
