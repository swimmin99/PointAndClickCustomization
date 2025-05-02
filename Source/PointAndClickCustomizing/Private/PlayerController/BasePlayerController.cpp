// Copyright 2025 Devhanghae All Rights Reserved.
#include "PlayerController/BasePlayerController.h"

#include "PointAndClickCustomizing.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Interface/CutomizableCharacter.h"
#include "Misc/Guid.h"


FName ABasePlayerController::GetPlayerKey() const
{
	if (!PlayerState)
	{
		UE_LOG(LogCustomizingPlugin, Warning, TEXT("ABasePlayerController::GetPlayerKey - PlayerState is null"));
		return NAME_None;
	}

	FUniqueNetIdRepl NetId = PlayerState->GetUniqueId();
	if (!NetId.IsValid())
	{
		UE_LOG(LogCustomizingPlugin, Warning, TEXT("ABasePlayerController::GetPlayerKey - Invalid UniqueNetId for %s"), *PlayerState->GetPlayerName());
		return NAME_None;
	}

	const FString IdString = NetId->ToString();
	UE_LOG(LogCustomizingPlugin, Log, TEXT("ABasePlayerController::GetPlayerKey - Returning PlayerKey=%s"), *IdString);
	return FName(*IdString);
}

void ABasePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (HasAuthority())
	{
		UE_LOG(LogCustomizingPlugin, Log, TEXT("ABasePlayerController::OnPossess - Authority detected, calling SetupAppearance"));
		SetupAppearance();
	}
}

void ABasePlayerController::SetupAppearance()
{
	if (APawn* PossessedPawn = GetPawn())
	{
		if (ICutomizableCharacter* Customizable = Cast<ICutomizableCharacter>(PossessedPawn))
		{
            UE_LOG(LogCustomizingPlugin, Log, TEXT("ABasePlayerController::SetupAppearance - Calling SetupPartsForCharacter"));
			Customizable->SetupPartsForCharacter(GetPlayerKey());
		}
		else
		{
            UE_LOG(LogCustomizingPlugin, Warning, TEXT("ABasePlayerController::SetupAppearance - Pawn does not implement ICustomizableCharacter"));
		}
	}
}



void ABasePlayerController::TravelToLevel(const FString& LevelName)
{
	if (HasAuthority())
	{
		UE_LOG(LogCustomizingPlugin, Log, TEXT("ABasePlayerController::TravelToLevel - ServerTravel to %s"), *LevelName);
		GetWorld()->ServerTravel(LevelName, true);
	}
	else
	{
		UE_LOG(LogCustomizingPlugin, Log, TEXT("ABasePlayerController::TravelToLevel - ClientTravel to %s"), *LevelName);
		ClientTravel(LevelName, TRAVEL_Absolute);
	}
}