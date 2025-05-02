// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PointAndClickCustomizing.h"    // for LogCustomizingPlugin
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "BasePlayerController.generated.h"

/**
 *  Base class for all player controllers.
 *  Provides helper methods for consistent PlayerID and level travel.
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Return a unique string key for this player. */
	UFUNCTION(BlueprintCallable, Category="CustomizingPlugin")
	FName GetPlayerKey() const;

	/** Called when possessing a Pawn; sets up appearance on server. */
	virtual void OnPossess(APawn* InPawn) override;

	/** Invoke customizable interface to set up attachments. */
	UFUNCTION(BlueprintCallable, Category="CustomizingPlugin")
	virtual void SetupAppearance();

	/** Travel locally or on server depending on authority. */
	UFUNCTION(BlueprintCallable, Category="CustomizingPlugin")
	void TravelToLevel(const FString& LevelName);
};
