// Copyright 2025 Devhanghae All Rights Reserved.
#include "ActorComponent/AttachmentFocusComponent.h"
#include "Actor/AttachableActor.h"
#include "GameFramework/PlayerController.h"
#include "PointAndClickCustomizing.h"
#include "ActorComponent/AttachmentRotationComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ActorComponent/StateMachineComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Character/CustomCharacter.h"
#include "Data/AttachmentDataStore.h"

UAttachmentFocusComponent::UAttachmentFocusComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    RotationComp    = CreateDefaultSubobject<UAttachmentRotationComponent>(TEXT("RotationComp"));
}
void UAttachmentFocusComponent::UpdateFocusDetection()
{
    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
        return;
    }

    FVector WorldOrigin, WorldDir;
    if (!PC->DeprojectMousePositionToWorld(WorldOrigin, WorldDir))
    {
        GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
        return;
    }

    const float TraceDistance = 10000.f;
    FVector Start = WorldOrigin;
    FVector End   = Start + WorldDir * TraceDistance;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());
    
#if WITH_EDITOR
    DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.f, 0, 1.f);
#endif

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        if (AAttachableActor* A = Cast<AAttachableActor>(Hit.GetActor()))
        {
            CachedCanFocusActor = A;
            GetOrCacheStateMachine()->SetState(ECustomizingState::ActorCanFocus);
            UE_LOG(LogCustomizingPlugin, Warning, TEXT("Hit Actor: %s"), *A->GetName());
            return;
        }
    }

    CachedCanFocusActor = nullptr;
    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
}

void UAttachmentFocusComponent::RotateFocusedActor(
    const FVector2D& PrevScreen,
    const FVector2D& CurrScreen,
    const FVector2D& ViewSize,
    float Speed
    )
{
    if (AAttachableActor* Actor = GetFocusedActor())
    {
        RotationComp->HandleRotation(PrevScreen, CurrScreen, ViewSize, Speed, GetFocusedActor());
        GetOrCacheStateMachine()->SetState(ECustomizingState::ActorFocused);
    }
}

void UAttachmentFocusComponent::EndRotate(FName PlayerID)
{
    RotationComp->EndAndSaveRotation(GetFocusedActor(), PlayerID);
}

bool UAttachmentFocusComponent::TryFocusAttachedActor()
{
    if (GetOrCacheStateMachine()->GetState() != ECustomizingState::ActorCanFocus
        || !CachedCanFocusActor)
    {
        return false;
    }

    FocusedActor = CachedCanFocusActor;
    GetOrCacheStateMachine()->SetState(ECustomizingState::ActorFocused);

    SetCurrentRecord({
        FocusedActor->ActorID,
        FocusedActor->GetAttachParentSocketName(),
        FRotator::ZeroRotator
    });
    ZoomIn();
    CachedCanFocusActor = nullptr;
    return true;
}

void UAttachmentFocusComponent::CancelFocus()
{
    FocusedActor = nullptr;
    CachedCanFocusActor = nullptr;
    ZoomOut();
    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
}

void UAttachmentFocusComponent::DeleteFocusedActor(FName PlayerKey)
{
    if (GetOrCacheStateMachine()->GetState() != ECustomizingState::ActorFocused || !FocusedActor)
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("DeleteFocusedActor - No focused actor to delete."));
        return;
    }

    Server_RequestRemoveAttachment(FocusedActor->ActorID, FocusedActor->BoneName, PlayerKey);
}

void UAttachmentFocusComponent::Client_ConfirmRemoveAttachment_Implementation(bool bWasSuccessful, FName ActorID, FName BoneID)
{
    if(!bWasSuccessful)
    {
        return;
    }
    if (GetOrCacheStateMachine()->GetState() != ECustomizingState::ActorFocused
        || !FocusedActor)
    {
        return;
    }
    if (ActorID != FocusedActor->ActorID || BoneID != FocusedActor->BoneName)
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("Client_ConfirmRemoveAttachment - Mismatched ActorID or BoneID for"));
        return;
    }
    FocusedActor->Destroy();
    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
    ZoomOut();
}

void UAttachmentFocusComponent::Server_RequestRemoveAttachment_Implementation(FName ActorID, FName BoneID, FName PlayerID)
{
    const bool bSuccess = UAttachmentDataStore::Get()->RemoveAttachment(ActorID, BoneID, PlayerID);
    Client_ConfirmRemoveAttachment(bSuccess, ActorID, BoneID);
    
    if (bSuccess)
    {
        UE_LOG(LogCustomizingPlugin, Log, TEXT("Server_RequestRemoveAttachment - Removed attachment successfully."));
    }
    else
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("Server_RequestRemoveAttachment - Failed to remove attachment from data store."));
    }
}

void UAttachmentFocusComponent::ZoomIn()
{
    if (bHasStoredCameraState) return;
    auto* PC   = Cast<APlayerController>(GetOwner()); if (!PC) return;
    auto* Char = Cast<ACustomCharacter>(PC->GetPawn());  if (!Char) return;
    auto* Spring = Char->SpringArmComp;                  if (!Spring) return;
    auto* Cam    = Char->CameraComp;                     if (!Cam)    return;

    OriginalSpringWorldLoc = Spring->GetComponentLocation();
    OriginalSpringRelLoc   = Spring->GetRelativeLocation();
    bHasStoredCameraState  = true;
    
    OriginalOrthoWidth = Cam->OrthoWidth;
    Cam->OrthoWidth = ZoomOrthoWidth;

    if (AAttachableActor* A = GetFocusedActor())
    {
        FVector NewWorldLoc = OriginalSpringWorldLoc;
        FVector ActorLoc    = A->GetActorLocation();
        NewWorldLoc.X = ActorLoc.X;
        NewWorldLoc.Z = ActorLoc.Z;
        Spring->SetWorldLocation(NewWorldLoc);

        UE_LOG(LogCustomizingPlugin, Log,
            TEXT("ZoomIn: SpringArm moved to XZ=%s"),
            *FVector(NewWorldLoc.X,0,NewWorldLoc.Z).ToString());
    }
}

void UAttachmentFocusComponent::ZoomOut()
{
    if (!bHasStoredCameraState) return;
    auto* PC   = Cast<APlayerController>(GetOwner()); if (!PC) return;
    auto* Char = Cast<ACustomCharacter>(PC->GetPawn());  if (!Char) return;
    auto* Spring = Char->SpringArmComp;                  if (!Spring) return;
    auto* Cam    = Char->CameraComp;                     if (!Cam)    return;

    Cam->OrthoWidth = OriginalOrthoWidth;
    Spring->SetRelativeLocation(OriginalSpringRelLoc);
    bHasStoredCameraState = false;

    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("ZoomOut: SpringArm restored to %s/%f"),
        *OriginalSpringWorldLoc.ToString(), OriginalOrthoWidth);
}