// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ReadyGameMode.generated.h"

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
	void OnPlayerReady(FName PlayerID);
protected:
	void TryTravel();
virtual void BeginPlay() override;
	UPROPERTY()
	TSet<FName> ReadyPlayers;
	
};
