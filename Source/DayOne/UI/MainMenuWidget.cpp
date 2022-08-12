// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnPlayButtonClicked registered ---"));
		PlayButton->OnClicked.AddDynamic(this, &ThisClass::OnPlayButtonClicked);
	}
	if (SettingsButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnSettingsButtonClicked registered ---"));
		SettingsButton->OnClicked.AddDynamic(this, &ThisClass::OnSettingsButtonClicked);
	}
	if (ExitButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnExitButtonClicked registered ---"));
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
	}

}

void UMainMenuWidget::OnPlayButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("OnPlayButtonClicked"));
}

void UMainMenuWidget::OnSettingsButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("OnSettingsButtonClicked"));

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

	Request->OnProcessRequestComplete().BindUObject(this, &ThisClass::OnGetSettingsResponseReceived);
	Request->SetURL("https://jsonplaceholder.typicode.com/posts");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void UMainMenuWidget::OnExitButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("OnExitButtonClicked"));
}

void UMainMenuWidget::OnGetSettingsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bConnectedSuccessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
	UE_LOG(LogTemp, Display, TEXT("Title: %s"), *ResponseObj->GetStringField("title"));
}
