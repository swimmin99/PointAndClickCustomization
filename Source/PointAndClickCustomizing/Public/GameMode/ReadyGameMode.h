// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "ReadyGameMode.generated.h"

class ABasePlayerController;
/**
 * 
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API AReadyGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AReadyGameMode();
	UPROPERTY(EditAnywhere, Category="Ready")
	FString TravelURL = TEXT("/PointAndClickCustomizing/Level/BattleArea?listen");
	void HandlePlayerReadyRequest(ABasePlayerController* PlayerController, FName PlayerID);
	void HandlePlayerCancelRequest(ABasePlayerController* PlayerController, FName PlayerID);

	
protected:
	void CheckAllPlayersReadyAndTravel();
	bool bIsTraveling = false;
	void TryTravel();
	
virtual void BeginPlay() override;
	UPROPERTY()
	TSet<FName> ReadyPlayers;
	
};
