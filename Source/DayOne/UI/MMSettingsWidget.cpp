// Fill out your copyright notice in the Description page of Project Settings.


#include "MMSettingsWidget.h"

#include "Components/Button.h"

void UMMSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TestButton_1)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnTestButton_1Clicked registered ---"));
		TestButton_1->OnClicked.AddDynamic(this, &ThisClass::OnTestButton_1Clicked);
	}
	if (TestButton_2)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnTestButton_2Clicked registered ---"));
		TestButton_2->OnClicked.AddDynamic(this, &ThisClass::OnTestButton_2Clicked);
	}
	if (TestButton_3)
	{
		UE_LOG(LogTemp, Warning, TEXT("--- OnTestButton_3Clicked registered ---"));
		TestButton_3->OnClicked.AddDynamic(this, &ThisClass::OnTestButton_3Clicked);
	}
}

void UMMSettingsWidget::OnTestButton_1Clicked()
{
	GEngine->AddOnScreenDebugMessage(
		-1,
		15.f,
		FColor::Blue,
		FString::Printf(TEXT("OnTestButton_1Clicked"))
	);
}

void UMMSettingsWidget::OnTestButton_2Clicked()
{
	GEngine->AddOnScreenDebugMessage(
	-1,
	15.f,
	FColor::Blue,
	FString::Printf(TEXT("OnTestButton_2Clicked"))
);
}

void UMMSettingsWidget::OnTestButton_3Clicked()
{
	GEngine->AddOnScreenDebugMessage(
	-1,
	15.f,
	FColor::Blue,
	FString::Printf(TEXT("OnTestButton_3Clicked"))
);
}
