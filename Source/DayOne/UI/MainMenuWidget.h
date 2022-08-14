// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Interfaces/IHttpRequest.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PlayButton;
	UFUNCTION()
	void OnPlayButtonClicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SettingsButton;
	UFUNCTION()
	void OnSettingsButtonClicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ExitButton;
	UFUNCTION()
	void OnExitButtonClicked();

	void OnGetSettingsResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
};
