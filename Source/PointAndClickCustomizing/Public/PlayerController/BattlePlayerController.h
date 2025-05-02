// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BasePlayerController.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "BattlePlayerController.generated.h"


/**
 * PlayerController for the battle area.
 * Handles back-to-customizing input.
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API ABattlePlayerController : public ABasePlayerController
{
    GENERATED_BODY()

public:
    /** Map path to return to customizing area. */
    UPROPERTY(EditAnywhere, Category="CustomizingPlugin|Level")
    FString CustomizingMapPath = TEXT("/PointAndClickCustomizing/Level/CustomizingArea.CustomizingArea?listen");

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupInputComponent() override;
    
    UPROPERTY(EditAnywhere, Category="Input")
    UInputAction* IA_Back;
private:
    /** Callback for back action (ESC). */
    void Input_Back(const FInputActionValue& Value);
};