// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MMSettingsWidget.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UMMSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TestButton_1;
	UFUNCTION()
	void OnTestButton_1Clicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TestButton_2;
	UFUNCTION()
	void OnTestButton_2Clicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TestButton_3;
	UFUNCTION()
	void OnTestButton_3Clicked();
};
