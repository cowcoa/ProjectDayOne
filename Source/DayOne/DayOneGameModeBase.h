// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DayOneGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API ADayOneGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void InitGameState() override;

	//void HandleStartingNewPlayer(class APlayerController* NewPlayer);

private:
	void InitGameLift() const;

	class UGameUserSettings* GetGameUserSettings();
};
