// Copyright Epic Games, Inc. All Rights Reserved.


#include "DayOneGameModeBase.h"
#include "GameLiftServerSDK.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/KismetStringLibrary.h"
#include "GameFramework/PlayerController.h"

void ADayOneGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	/*
	UGameUserSettings* Settings = GetGameUserSettings(); // note we are using the function defined above
	if (Settings != nullptr)
	{
		Settings->SetFullscreenMode(EWindowMode::Windowed);
		Settings->ApplyResolutionSettings(true);
	}
	*/
}

void ADayOneGameModeBase::InitGameState()
{
	Super::InitGameState();
	UE_LOG(LogTemp, Warning, TEXT("InitGameState....!!!!-------"));
	//InitGameLift();
}

void ADayOneGameModeBase::InitGameLift() const
{
#if UE_SERVER
	UE_LOG(LogTemp, Warning, TEXT("InitGameLift in DS.....!!!!!+++++"));

	//Let's run this code only if GAMELIFT is enabled. Only with Server targets!
#if WITH_GAMELIFT

	//Getting the module first.
	FGameLiftServerSDKModule* gameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

	//InitSDK will establish a local connection with GameLift's agent to enable further communication.
	gameLiftSdkModule->InitSDK();

	//When a game session is created, GameLift sends an activation request to the game server and passes along the game session object containing game properties and other settings.
	//Here is where a game server should take action based on the game session object.
	//Once the game server is ready to receive incoming player connections, it should invoke GameLiftServerAPI.ActivateGameSession()
	auto onGameSession = [=](Aws::GameLift::Server::Model::GameSession gameSession)
	{
		gameLiftSdkModule->ActivateGameSession();
	};

	FProcessParameters* params = new FProcessParameters();
	params->OnStartGameSession.BindLambda(onGameSession);

	//OnProcessTerminate callback. GameLift will invoke this callback before shutting down an instance hosting this game server.
	//It gives this game server a chance to save its state, communicate with services, etc., before being shut down.
	//In this case, we simply tell GameLift we are indeed going to shutdown.
	params->OnTerminate.BindLambda([=]() {gameLiftSdkModule->ProcessEnding(); });

	//This is the HealthCheck callback.
	//GameLift will invoke this callback every 60 seconds or so.
	//Here, a game server might want to check the health of dependencies and such.
	//Simply return true if healthy, false otherwise.
	//The game server has 60 seconds to respond with its health status. GameLift will default to 'false' if the game server doesn't respond in time.
	//In this case, we're always healthy!
	params->OnHealthCheck.BindLambda([]() {return true; });

	//This game server tells GameLift that it will listen on port 7777 for incoming player connections.
	params->port = 7777;

	//Here, the game server tells GameLift what set of files to upload when the game session ends.
	//GameLift will upload everything specified here for the developers to fetch later.
	TArray<FString> logfiles;
	logfiles.Add(TEXT("aLogFile.txt"));
	params->logParameters = logfiles;

	//Calling ProcessReady tells GameLift this game server is ready to receive incoming game sessions!
	gameLiftSdkModule->ProcessReady(*params);
#endif

#endif
}

/*
void ADayOneGameModeBase::HandleStartingNewPlayer(APlayerController* NewPlayer)
{
	const FString& StartPointName = UKismetStringLibrary::GetSubstring(OptionsString, 1,(OptionsString.Len() - 1));
	UE_LOG(LogTemp, Warning, TEXT("OptionsString: %s, PlayerStartPoint: %s"), *OptionsString, *StartPointName);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Blue,
			FString::Printf(TEXT("OptionsString: %s, PlayerStartPoint: %s"), *OptionsString, *StartPointName)
		);
	}
	
	AActor* PlayerStartPoint = FindPlayerStart(NewPlayer, StartPointName);
	
	RestartPlayerAtPlayerStart(NewPlayer, PlayerStartPoint);
}
*/

UGameUserSettings* ADayOneGameModeBase::GetGameUserSettings()
{
	if (GEngine != nullptr)
	{
		return GEngine->GameUserSettings;
	}
	return nullptr;
}