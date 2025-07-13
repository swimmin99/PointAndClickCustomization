// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PointAndClickCustomizing.h"    
#include "Data/FPopupData.h"
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
	void TravelToLevel(const FString& LevelName);

	UFUNCTION(BlueprintCallable, Category="CustomizingPlugin")
	void OnPressReadyButton();

	UFUNCTION(Client, Reliable)
	void Client_ShowWaitingPopup();

	UFUNCTION(Client, Reliable)
	void Client_ClosePopup();

	UFUNCTION(Client, Reliable)
	void Client_ShowReadyConfirmationFailed();

	void RequestPopup(const FText& Title, const FText& Description, EPopupButtonType ButtonType, FPopupDelegate OnConfirmed = FPopupDelegate(), FPopupDelegate OnCancelled = FPopupDelegate());

protected:
	UFUNCTION(Server, Reliable)
	void Server_RequestReady(FName PlayerID);

	UFUNCTION(Server, Reliable)
	void Server_RequestCancelReady(FName PlayerID);

	void ShowReadyConfirmationPopup();
	
	UFUNCTION()
	void OnCancelWaiting();

};
