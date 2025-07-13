// Copyright 2025 Devhanghae All Rights Reserved.
#include "PlayerController/BasePlayerController.h"
#include "GameFramework/Pawn.h"
#include "PointAndClickCustomizing.h"
#include "Data/UITextConstants.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "GameMode/ReadyGameMode.h"
#include "Interface/CutomizableCharacter.h"
#include "Misc/Guid.h"
#include "Engine/GameInstance.h"
#include "UI/PopupManagerSubsystem.h"


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

void ABasePlayerController::RequestPopup(const FText& Title, const FText& Description, EPopupButtonType ButtonType, FPopupDelegate OnConfirmed, FPopupDelegate OnCancelled)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPopupManagerSubsystem* PopupManager = GameInstance->GetSubsystem<UPopupManagerSubsystem>())
		{
			FPopupData PopupData;
			PopupData.Title = Title;
			PopupData.Description = Description;
			PopupData.ButtonType = ButtonType;

			PopupData.OnConfirmed = OnConfirmed;
			PopupData.OnCancelled = OnCancelled;

			PopupManager->RequestShowPopup(PopupData);
		}
	}
}

void ABasePlayerController::OnPressReadyButton()
{
    if (!IsLocalController())
    {
        return;
    }
    ShowReadyConfirmationPopup();
}

void ABasePlayerController::ShowReadyConfirmationPopup()
{
    FPopupDelegate CancelDelegate;
    CancelDelegate.BindUFunction(this, FName("OnCancelWaiting"));
	RequestPopup(
        UUITextConstants::GetConfirmReady_Title(),
        UUITextConstants::GetConfirmReady_Description(),
        EPopupButtonType::CancelOnly,
        CancelDelegate
    );
}



void ABasePlayerController::OnCancelWaiting()
{
    Server_RequestCancelReady(GetPlayerKey());
}


void ABasePlayerController::Client_ShowWaitingPopup_Implementation()
{
    FPopupDelegate CancelDelegate;
    CancelDelegate.BindUFunction(this, FName("OnCancelWaiting"));

    RequestPopup(
        UUITextConstants::GetWaitingForPlayers_Title(),
        UUITextConstants::GetWaitingForPlayers_Description(),
        EPopupButtonType::CancelOnly,
        FPopupDelegate(),
        CancelDelegate
    );
}

void ABasePlayerController::Client_ClosePopup_Implementation()
{
   
}

void ABasePlayerController::Client_ShowReadyConfirmationFailed_Implementation()
{
    Client_ShowWaitingPopup();
}

void ABasePlayerController::Server_RequestReady_Implementation(FName PlayerID)
{
    if (AReadyGameMode* GM = GetWorld()->GetAuthGameMode<AReadyGameMode>())
    {
        GM->HandlePlayerReadyRequest(this, PlayerID);
    }
}

void ABasePlayerController::Server_RequestCancelReady_Implementation(FName PlayerID)
{
	UE_LOG(LogTemp, Warning, TEXT("[HandlePlayerCancelRequest] RPC Called for ServerRPC ."));

    if (AReadyGameMode* GM = GetWorld()->GetAuthGameMode<AReadyGameMode>())
    {
    	UE_LOG(LogTemp, Warning, TEXT("[HandlePlayerCancelRequest] Requested Cancel for GM (Server)."));
        GM->HandlePlayerCancelRequest(this, PlayerID);
    }
}
