// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterNameWidget.h"

void UCharacterNameWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UCharacterNameWidget::SetDisplayText(FString Text)
{
	if (CharacterName)
	{
		CharacterName->SetText(FText::FromString(Text));
		UE_LOG(LogTemp, Warning, TEXT("DisplayText is %s"), *Text);
	}
}

void UCharacterNameWidget::DebugShowLocalNetRole(APawn* Character)
{
	if (Character == nullptr) return;

	ENetRole LocalRole = Character->GetLocalRole();
	FString RoleText;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority:
		RoleText = "LocalRole: Authority";
		break;
	case ENetRole::ROLE_AutonomousProxy:
		RoleText = "LocalRole: AutonomousProxy";
		break;
	case ENetRole::ROLE_SimulatedProxy:
		RoleText = "LocalRole: SimulatedProxy";
		break;
	default:
		RoleText = "LocalRole: None";
		break;
	}

	UE_LOG(LogTemp, Warning, TEXT("RoleText is %s"), *RoleText);
	SetDisplayText(RoleText);
}
