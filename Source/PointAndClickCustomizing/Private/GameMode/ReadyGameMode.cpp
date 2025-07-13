// Copyright 2025 Devhanghae All Rights Reserved.
#include "GameMode/ReadyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "PlayerController/BasePlayerController.h"
#include "TimerManager.h"


AReadyGameMode::AReadyGameMode()
{
	ReadyPlayers.Empty();
	UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] Initialized. TravelURL=%s"), *TravelURL);
}

void AReadyGameMode::HandlePlayerReadyRequest(ABasePlayerController* PlayerController, FName PlayerID)
{
	if (!PlayerController || PlayerID.IsNone() || bIsTraveling)
	{
		return;
	}

	if (ReadyPlayers.Contains(PlayerID)) return;

	ReadyPlayers.Add(PlayerID);
	UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] PlayerID=%s is now ready. Total: %d"), *PlayerID.ToString(), ReadyPlayers.Num());

	PlayerController->Client_ShowWaitingPopup();

	CheckAllPlayersReadyAndTravel();
}

void AReadyGameMode::HandlePlayerCancelRequest(ABasePlayerController* PlayerController, FName PlayerID)
{
	if (!PlayerController || PlayerID.IsNone())
	{
		return;
	}
	if (bIsTraveling)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ReadyGameMode] PlayerID=%s tried to cancel, but travel has already started."), *PlayerID.ToString());
		PlayerController->Client_ShowReadyConfirmationFailed();
		return;
	}

	const int32 RemovedCount = ReadyPlayers.Remove(PlayerID);
	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] PlayerID=%s cancelled ready. Total: %d"), *PlayerID.ToString(), ReadyPlayers.Num());
	}
}


void AReadyGameMode::CheckAllPlayersReadyAndTravel()
{
	if (bIsTraveling) return;

	int32 TotalPlayers = GameState ? GameState->PlayerArray.Num() : 0;
	if (TotalPlayers > 0 && ReadyPlayers.Num() >= TotalPlayers)
	{
		UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] All players are ready. Traveling to %s"), *TravelURL);
		bIsTraveling = true; 

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ABasePlayerController* PC = Cast<ABasePlayerController>(It->Get()))
			{
				PC->Client_ClosePopup();
			}
		}
        
		FTimerHandle TravelTimer;
		GetWorldTimerManager().SetTimer(TravelTimer, this, &AReadyGameMode::TryTravel, 1.0f, false);
	}
}

void AReadyGameMode::TryTravel()
{
	GetWorld()->ServerTravel(TravelURL);
}

void AReadyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

