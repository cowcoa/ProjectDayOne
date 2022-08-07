// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "HttpModule.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/IHttpResponse.h"

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

void ALobbyGameMode::StartPlay()
{
	Super::StartPlay();

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	/*Request->OnProcessRequestComplete().BindUObject(this, &AHttpTestGameMode::OnResponseReceived);
	  Request->SetURL("https://jsonplaceholder.typicode.com/posts/1");
	  Request->SetVerb("GET");
	  Request->ProcessRequest();*/
  
	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("title", "foo");

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::OnResponseReceived);
	Request->SetURL("https://jsonplaceholder.typicode.com/posts");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ALobbyGameMode::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
	UE_LOG(LogTemp, Display, TEXT("Title: %s"), *ResponseObj->GetStringField("title"));
}
