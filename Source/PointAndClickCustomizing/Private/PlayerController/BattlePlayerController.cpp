// Copyright 2025 Devhanghae All Rights Reserved.
#include "PlayerController/BattlePlayerController.h"

ABattlePlayerController::ABattlePlayerController()
{
}

void ABattlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}