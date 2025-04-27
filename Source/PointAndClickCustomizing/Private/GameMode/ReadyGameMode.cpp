// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ReadyGameMode.h"

#include "GameFramework/GameStateBase.h"


AReadyGameMode::AReadyGameMode()
{
	ReadyPlayers.Empty();
	UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] Initialized. TravelURL=%s"), *TravelURL);
}

void AReadyGameMode::OnPlayerReady(FName PlayerID)
{
	UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] OnPlayerReady called. PlayerID=%s"), *PlayerID.ToString());

	// 중복 방지 및 추가
	ReadyPlayers.Add(PlayerID);
	UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] Added Player. Current ready count: %d"), ReadyPlayers.Num());

	int32 TotalPlayers = 0;
	if (GameState)
	{
		TotalPlayers = GameState->PlayerArray.Num();
		UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] Total players (incl. host): %d"), TotalPlayers);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ReadyGameMode] GameState is null! Can't determine total players."));
	}

	int32 RequiredCount = FMath::Max(TotalPlayers, 0);
	UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] Players needed to travel (excl. host): %d"), RequiredCount);

	if (ReadyPlayers.Num() >= RequiredCount)
	{
		UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] All players ready. Executing travel."));
		TryTravel();
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] Waiting for more players to be ready."));
	}
}

void AReadyGameMode::TryTravel()
{
	if (GetWorld())
	{
		UE_LOG(LogTemp, Log, TEXT("[ReadyGameMode] Traveling to: %s"), *TravelURL);
		GetWorld()->ServerTravel(TravelURL);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ReadyGameMode] TryTravel failed: World is null."));
	}
}

void AReadyGameMode::BeginPlay()
{
	Super::BeginPlay();
}

