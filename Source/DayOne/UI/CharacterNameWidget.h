// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "CharacterNameWidget.generated.h"

/**
 * 
 */
UCLASS()
class DAYONE_API UCharacterNameWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CharacterName;
	
protected:
	// virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

public:
	UFUNCTION(BlueprintCallable)
	void SetDisplayText(FString Text);

	UFUNCTION(BlueprintCallable)
	void DebugShowLocalNetRole(APawn* Character);
};
