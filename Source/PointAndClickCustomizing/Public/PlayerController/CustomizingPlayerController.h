// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PointAndClickCustomizing.h"           // for LogCustomizingPlugin
#include "BasePlayerController.h"
#include "InputActionValue.h"
#include "Data/FAttachmentRecord.h"
#include "UI/CustomizingMainHUD.h" 
#include "CustomizingPlayerController.generated.h"

class UCustomizingGatewayComponent;
class UInputMappingContext;
class UInputAction;

/**
 * PlayerController for the customization area.
 * Handles input mapping, spawning/attaching, and ready-state signaling.
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API ACustomizingPlayerController : public ABasePlayerController
{
    GENERATED_BODY()

public:
    ACustomizingPlayerController();

    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupInputComponent() override;

    
    /** Manages preview, focus, attach logic. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CustomizingPlugin")
    UCustomizingGatewayComponent* CustomizingComp;
    
    /** Spawn a preview actor for the given ID. */
    UFUNCTION(BlueprintCallable, Category="CustomizingPlugin")
    void TrySpawningAttachableActor(FName ID);

    /** Press Ready button to notify server. */
    UFUNCTION(BlueprintCallable, Category="CustomizingPlugin")
    void PressReadyButton();

    /** Send ready-state RPC. */
    UFUNCTION(Server, Reliable, Category="CustomizingPlugin")
    void Server_SendReady(FName PlayerID);

    /** Maini HUD Class  */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UCustomizingMainHUD> MainHUDWidgetClass;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override
    {
        Super::GetLifetimeReplicatedProps(Out);
        // No additional replicated props.
    }

    /** Enhanced Input mapping context. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Input")
    UInputMappingContext* CustomizingMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Input")
    UInputAction* IA_Look;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Input")
    UInputAction* IA_Zoom;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Input")
    UInputAction* IA_LeftClick;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Input")
    UInputAction* IA_RightClick;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Input")
    UInputAction* IA_Delete;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Input")
    UInputAction* IA_Back;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Input")
    UInputAction* IA_Enter;
  

    /** Rotation speed multiplier. */
    UPROPERTY(EditAnywhere, Category="CustomizingPlugin")
    float RotationSpeed = 1.5f;

    /** Zoom speed multiplier. */
    UPROPERTY(EditAnywhere, Category="CustomizingPlugin")
    float ZoomSpeed = 50.f;

    /** Min orthographic width. */
    UPROPERTY(EditAnywhere, Category="CustomizingPlugin")
    float MinZoom = 256.f;

    /** Max orthographic width. */
    UPROPERTY(EditAnywhere, Category="CustomizingPlugin")
    float MaxZoom = 1024.f;

private:
    // Interpolation target for ortho width.
    float TargetOrthoWidth = 512.f;


    // Saved positions for drag deltas.
    FVector2D StoredLeftMouseDownPos = FVector2D::ZeroVector;
    FVector2D StoredRightMouseDownPos = FVector2D::ZeroVector;

    // ---- Input callbacks ----
    void Input_Look(const FInputActionValue& Value);
    void HandleCharacterRotation(const FInputActionValue& Value);
    void HandleSelectedActorRotation(const FInputActionValue& Value);
    void Input_Zoom(const FInputActionValue& Value);
    void Input_LeftClick(const FInputActionValue& Value);
    void Input_LeftClickEnded(const FInputActionValue& Value);
    void Input_Delete(const FInputActionValue& Value);
    void Input_Back(const FInputActionValue& Value);
    void Input_GoToBattle(const FInputActionValue& Value);
};