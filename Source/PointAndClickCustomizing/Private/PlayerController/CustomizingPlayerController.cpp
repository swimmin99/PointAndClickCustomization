// Copyright 2025 Devhanghae All Rights Reserved.

#include "PlayerController/CustomizingPlayerController.h"
#include "PointAndClickCustomizing.h"             
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Actor/AttachableActor.h"
#include "ActorComponent/CustomizingGatewayComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/CustomCharacter.h"
#include "GameMode/ReadyGameMode.h"
#include "Engine/LocalPlayer.h"
#include "UI/CustomizingMainHUD.h"
#include "Data/AttachmentDataStore.h"

ACustomizingPlayerController::ACustomizingPlayerController()
{
    CustomizingComp = CreateDefaultSubobject<UCustomizingGatewayComponent>(TEXT("CustomizingGatewayComponent"));
    UE_LOG(LogCustomizingPlugin, Log, TEXT("ACustomizingPlayerController::Constructor"));
}

void ACustomizingPlayerController::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogCustomizingPlugin, Log, TEXT("ACustomizingPlayerController::BeginPlay"));

    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
        {
            if (CustomizingMappingContext)
            {
                Sub->AddMappingContext(CustomizingMappingContext, 1);
                UE_LOG(LogCustomizingPlugin, Log, TEXT("Added Enhanced Input mapping context"));
            }
        }
    }

    // Enable mouse events for UI interaction
    bShowMouseCursor       = true;
    bEnableClickEvents     = true;
    bEnableMouseOverEvents = true;

    if (IsLocalPlayerController())
    {
        if (MainHUDWidgetClass)
        {
            UCustomizingMainHUD* MyHud = CreateWidget<UCustomizingMainHUD>(this, MainHUDWidgetClass);
            if (MyHud)
            {
                FSimpleDelegate Delegate;
                Delegate.BindUObject(this, &ACustomizingPlayerController::PressReadyButton);
                MyHud->SetOnReadyButtonClickedDelegate(Delegate, FText::FromString(TEXT("Ready")));

                MyHud->AddToViewport();
                UE_LOG(LogCustomizingPlugin, Log, TEXT("Main HUD created and added to viewport."));
            }
            else
            {
                UE_LOG(LogCustomizingPlugin, Error, TEXT("Failed to create Main HUD widget instance."));
            }
        }
        else
        {
            UE_LOG(LogCustomizingPlugin, Error, TEXT("MainHUDWidgetClass is not set in PlayerController. Cannot create Main HUD."));
        }
    }
}

void ACustomizingPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
}

void ACustomizingPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    UE_LOG(LogCustomizingPlugin, Log, TEXT("ACustomizingPlayerController::SetupInputComponent"));

    // Ensure mouse interaction
    bShowMouseCursor       = true;
    bEnableClickEvents     = true;
    bEnableMouseOverEvents = true;
    FInputModeGameAndUI Mode;
    Mode.SetHideCursorDuringCapture(false);
    SetInputMode(Mode);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
    {
        EIC->BindAction(IA_LeftClick,  ETriggerEvent::Started,   this, &ThisClass::Input_LeftClick);
        EIC->BindAction(IA_LeftClick,  ETriggerEvent::Completed,   this, &ThisClass::Input_LeftClickEnded);
        EIC->BindAction(IA_Look,       ETriggerEvent::Triggered, this, &ThisClass::Input_Look);
        EIC->BindAction(IA_Zoom,       ETriggerEvent::Triggered, this, &ThisClass::Input_Zoom);
        EIC->BindAction(IA_RightClick, ETriggerEvent::Triggered, this, &ThisClass::Input_Back);
        EIC->BindAction(IA_Delete,     ETriggerEvent::Triggered, this, &ThisClass::Input_Delete);
        EIC->BindAction(IA_Back,       ETriggerEvent::Triggered, this, &ThisClass::Input_Back);
        EIC->BindAction(IA_Enter,      ETriggerEvent::Triggered, this, &ThisClass::Input_GoToBattle);
    }
    else
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("Missing EnhancedInputComponent"));
    }
}

void ACustomizingPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const ECustomizingState State = CustomizingComp ? CustomizingComp->GetState() : ECustomizingState::Idle;

    if (APawn* P = GetPawn())
    if (UCameraComponent* Cam = P->FindComponentByClass<UCameraComponent>())
    if (Cam->ProjectionMode == ECameraProjectionMode::Orthographic && State != ECustomizingState::ActorFocused)
    {
        Cam->OrthoWidth = FMath::FInterpTo(Cam->OrthoWidth, TargetOrthoWidth, DeltaSeconds, 5.f);
    }
}

void ACustomizingPlayerController::TrySpawningAttachableActor(FName ID)
{
    if (!CustomizingComp || !CustomizingComp->RequestSpawnByID(ID))
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("TrySpawningAttachableActor - Failed to spawn %s"), *ID.ToString());
    }
}

void ACustomizingPlayerController::PressReadyButton()
{
    if (!IsLocalController()) return;
    const FName MyID = GetPlayerKey();
    UE_LOG(LogCustomizingPlugin, Log, TEXT("PressReadyButton - PlayerID=%s"), *MyID.ToString());
    Server_SendReady(MyID);
}

// ---- Server RPCs ----

void ACustomizingPlayerController::Server_SendReady_Implementation(FName PlayerID)
{
    if (AReadyGameMode* GM = GetWorld()->GetAuthGameMode<AReadyGameMode>())
    {
        GM->OnPlayerReady(PlayerID);
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Server_SendReady - Notified GameMode of PlayerID=%s"), *PlayerID.ToString());
    }
}


// ---- Input Callbacks ----

void ACustomizingPlayerController::Input_LeftClick(const FInputActionValue& Value)
{
    const ECustomizingState State = CustomizingComp ? CustomizingComp->GetState() : ECustomizingState::Idle;

    if (State == ECustomizingState::ActorCanFocus)
    {
        CustomizingComp->TryFocusAttachedActor();
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Input_LeftClick - TryFocusAttachedActor"));
        return;
    }

    if (State == ECustomizingState::ActorSnapped && IsLocalController())
    {
        CustomizingComp->FinalizeAttachment(GetPlayerKey());
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Input_LeftClick - Finalized and added attachment"));
    }

    GetMousePosition(StoredLeftMouseDownPos.X, StoredLeftMouseDownPos.Y);
}

void ACustomizingPlayerController::Input_LeftClickEnded(const FInputActionValue& Value)
{
    const ECustomizingState State = CustomizingComp ? CustomizingComp->GetState() : ECustomizingState::Idle;
    if (State == ECustomizingState::ActorFocused)
    {
        CustomizingComp->TrySaveRotation(GetPlayerKey());
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Input_LeftClick Ended - TrySaveRotation"));
        return;
    }
}

void ACustomizingPlayerController::Input_Look(const FInputActionValue& Value)
{
    if (!CustomizingComp) {
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Input_Look - CustomizingComp null"));
        return; }
    const FVector2D Delta = Value.Get<FVector2D>();
    const ECustomizingState State = CustomizingComp->GetState();

    if (State == ECustomizingState::ActorFocused)
    {
        HandleSelectedActorRotation(Value);
        UE_LOG(LogCustomizingPlugin, Verbose, TEXT("Input_Look - Rotated focused actor by %s"), *Delta.ToString());
    }
    else
    {
        HandleCharacterRotation(Value);
        UE_LOG(LogCustomizingPlugin, Verbose, TEXT("Input_Look - Rotated Character by %s"), *Delta.ToString());
    }
}

void ACustomizingPlayerController::HandleCharacterRotation(const FInputActionValue& Value)
{
    FVector2D Curr; GetMousePosition(Curr.X, Curr.Y);
    const float Dir = (Curr.X < StoredLeftMouseDownPos.X) ? -1.f : 1.f;
    if (ACustomCharacter* C = Cast<ACustomCharacter>(GetPawn()))
        if (USkeletalMeshComponent* Mesh = C->GetCustomizingMesh())
        {
            FRotator R = Mesh->GetRelativeRotation();
            R.Yaw += Dir * RotationSpeed;
            Mesh->SetRelativeRotation(R);
            UE_LOG(LogCustomizingPlugin, Verbose, TEXT("Input_Look - Rotated mesh by %f"), Dir * RotationSpeed);
        }
    StoredLeftMouseDownPos = Curr;
}
void ACustomizingPlayerController::HandleSelectedActorRotation(const FInputActionValue& Value)
{
    FVector2D Curr; 
    GetMousePosition(Curr.X, Curr.Y);

    int32 ViewportX, ViewportY;
    GetViewportSize(ViewportX, ViewportY);

    FVector2D ViewportSize((float)ViewportX, (float)ViewportY);

    CustomizingComp->RotateFocusedActor(
        StoredLeftMouseDownPos,
        Curr,
        ViewportSize,
        RotationSpeed
    );

    UE_LOG(LogCustomizingPlugin, Verbose,
           TEXT("TrackballRotate - From=(%f,%f) Viewport=(%d,%d)"),
           StoredLeftMouseDownPos.X, StoredLeftMouseDownPos.Y,
           ViewportX, ViewportY
    );

    StoredLeftMouseDownPos = Curr;
}


void ACustomizingPlayerController::Input_Zoom(const FInputActionValue& Value)
{
    const float Z = Value.Get<float>();
    if (FMath::IsNearlyZero(Z)) return;

    if (APawn* P = GetPawn())
    if (UCameraComponent* Cam = P->FindComponentByClass<UCameraComponent>())
    if (Cam->ProjectionMode == ECameraProjectionMode::Orthographic)
    {
        TargetOrthoWidth = FMath::Clamp(TargetOrthoWidth - Z * ZoomSpeed, MinZoom, MaxZoom);
        UE_LOG(LogCustomizingPlugin, Verbose, TEXT("Input_Zoom - TargetOrthoWidth=%.1f"), TargetOrthoWidth);
    }
}

void ACustomizingPlayerController::Input_Delete(const FInputActionValue& Value)
{
    if (!CustomizingComp) return;
    if (CustomizingComp->GetState() == ECustomizingState::ActorFocused)
    {
        CustomizingComp->DeleteFocusedActor(GetPlayerKey());
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Input_Delete - Deleted focused actor"));
    }
}

void ACustomizingPlayerController::Input_Back(const FInputActionValue& Value)
{
    if (!CustomizingComp) return;
    ECustomizingState State = CustomizingComp->GetState();
    if (State == ECustomizingState::ActorMoving)
    {
        CustomizingComp->CancelPreview();
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Input_Back - CancelPreview"));
    }
    else if (State == ECustomizingState::ActorFocused)
    {
        CustomizingComp->CancelFocus();
        if (APawn* P = GetPawn())
        if (UCameraComponent* Cam = P->FindComponentByClass<UCameraComponent>())
            TargetOrthoWidth = Cam->OrthoWidth;
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Input_Back - CancelFocus"));
    }
}

void ACustomizingPlayerController::Input_GoToBattle(const FInputActionValue& Value)
{
    UE_LOG(LogCustomizingPlugin, Log, TEXT("Input_GoToBattle - Preparing to send ready"));
    PressReadyButton();
}
