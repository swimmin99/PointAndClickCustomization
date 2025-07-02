#include "ActorComponent/AttachmentPreviewComponent.h"
#include "Actor/AttachableActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PointAndClickCustomizing.h"
#include "ActorComponent/StateMachineComponent.h"
#include "Character/CustomCharacter.h"
#include "Data/AttachmentDataStore.h"
// Copyright 2025 Devhanghae All Rights Reserved.
#include "PlayerController/CustomizingPlayerController.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

UAttachmentPreviewComponent::UAttachmentPreviewComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UAttachmentPreviewComponent::RequestSpawnByID(FName ActorID)
{
    if (!GetOrCacheCharacter() || !GetOrCacheActorDataTable())
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("RequestSpawnByID failed: Character=%s, Table=%s"),
            *FString(GetOrCacheCharacter() ? TEXT("Valid") : TEXT("Null")),
            *FString(GetOrCacheActorDataTable() ? TEXT("Valid") : TEXT("Null")));
        return false;
    }

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }

    PreviewActor = AAttachableActor::SpawnPreview(ActorID, GetOrCacheActorDataTable(), this);
    if (!PreviewActor)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnPreview returned null for ID=%s"), *ActorID.ToString());
        return false;
    }


    GetOrCacheStateMachine()->SetState(ECustomizingState::ActorMoving);
    PreviewActor->SetReplicates(false);
    TargetLocation = PreviewActor->GetActorLocation();
    return true;
}

void UAttachmentPreviewComponent::FinalizeAttachment(FName PlayerID)
{

        FAttachmentRecord Rec;
        Rec.ActorID          = PreviewActor->ActorID;
        Rec.BoneName         = CurrentSnapBone;
        FTransform SockT     = GetOrCacheMesh()->GetSocketTransform(CurrentSnapBone, RTS_Component);
        Rec.ActorRotation = (PreviewActor->GetActorQuat() * SockT.GetRotation().Inverse()).Rotator();
        CurrentRecord = Rec;

        AAttachableActor* TargetActor = PreviewActor;
        TargetActor->BoneName = Rec.BoneName;
        
        Server_SaveAttachmentRecord(CurrentRecord, PlayerID);
}

void UAttachmentPreviewComponent::CancelPreview()
{
    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }
    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
}

void UAttachmentPreviewComponent::Update(float DeltaTime)
{
    if (!PreviewActor || !GetOrCacheCharacter())
        return;

    if (IsPreviewNearBone())
        GetOrCacheStateMachine()->SetState(ECustomizingState::ActorSnapped);
    else
        GetOrCacheStateMachine()->SetState(ECustomizingState::ActorMoving);

    if (GetOrCacheStateMachine()->GetState() == ECustomizingState::ActorMoving)
        UpdateMoving();
    else
        FixLocation();

    FVector Current = PreviewActor->GetActorLocation();
    FVector NewLoc  = FMath::VInterpTo(Current, TargetLocation, DeltaTime, MoveInterpSpeed);
    PreviewActor->SetActorLocation(NewLoc);
}

bool UAttachmentPreviewComponent::IsPreviewNearBone()
{
    FVector Loc = GetMouseIntersectionLoc();
    if (auto* Mesh = GetOrCacheMesh())
    {
        CurrentSnapBone = Mesh->FindClosestBone(Loc, &ClosestBoneLoc);
        return FVector::Dist(Loc, ClosestBoneLoc) <= SnapThreshold;
    }
    return false;
}

void UAttachmentPreviewComponent::UpdateMoving()
{
    TargetLocation = GetMouseIntersectionLoc();
}

void UAttachmentPreviewComponent::FixLocation()
{
    TargetLocation = ClosestBoneLoc;
}

FVector UAttachmentPreviewComponent::GetMouseIntersectionLoc()
{
    if (auto* PC = Cast<APlayerController>(GetOwner()))
    {
        FVector2D Screen;
        PC->GetMousePosition(Screen.X, Screen.Y);

        FVector WorldLoc, WorldDir;
        if (UGameplayStatics::DeprojectScreenToWorld(PC, Screen, WorldLoc, WorldDir))
        {
            if (auto* Mesh = GetOrCacheMesh())
            {
                const FVector PlanePoint  = Mesh->GetComponentLocation();
                const FVector PlaneNormal = FVector::CrossProduct(
                    FVector::UpVector,
                    GetOrCacheCharacter()->GetCustomizingForwardVector()
                ).GetSafeNormal();

                return FMath::LinePlaneIntersection(
                    WorldLoc,
                    WorldLoc + WorldDir * 10000.f,
                    PlanePoint,
                    PlaneNormal
                );
            }
        }
    }
    return FVector::ZeroVector;
}

void UAttachmentPreviewComponent::UpdateDebug()
{
#if WITH_EDITOR
    if (auto* Mesh = GetOrCacheMesh())
    {
        DrawDebugSphere(
            GetWorld(),
            Mesh->GetComponentLocation(),
            25.f, 12,
            FColor::Blue,
            false, 0.1f
        );
    }
#endif
}


void UAttachmentPreviewComponent::Client_ConfirmAddAttachment_Implementation(bool bWasSuccessful, const FAttachmentRecord& ConfirmedRecord)
{
    if (auto* PC = Cast<APlayerController>(GetOwner()))
    {
        UE_LOG(LogCustomizingPlugin, Log, TEXT("[%s] Client_ConfirmAddAttachment received. Success: %s, ActorID: %s, Bone: %s"), 
            *PC->GetName(), bWasSuccessful ? TEXT("true") : TEXT("false"), *ConfirmedRecord.ActorID.ToString(), *ConfirmedRecord.BoneName.ToString());
    }

    if (bWasSuccessful)
    {
        if (GetOrCacheStateMachine()->GetState() != ECustomizingState::ActorSnapped)
        {
            UE_LOG(LogCustomizingPlugin, Warning, TEXT("Client_ConfirmAddAttachment: State was not 'ActorSnapped' (%s). Ignoring confirmation."), *UEnum::GetValueAsString(GetOrCacheStateMachine()->GetState()));
            return;
        }

        if (!PreviewActor)
        {
            UE_LOG(LogCustomizingPlugin, Error, TEXT("Client_ConfirmAddAttachment: bWasSuccessful is true, but PreviewActor is null. Cannot attach."));
            return;
        }

        if (PreviewActor->ActorID != ConfirmedRecord.ActorID)
        {
            UE_LOG(LogCustomizingPlugin, Error, TEXT("Client_ConfirmAddAttachment: Mismatched ActorID. Previewing '%s' but server confirmed '%s'."), *PreviewActor->ActorID.ToString(), *ConfirmedRecord.ActorID.ToString());
            CancelPreview(); 
            return;
        }
        
        if (auto* Mesh = GetOrCacheMesh())
        {
            UE_LOG(LogCustomizingPlugin, Log, TEXT("All checks passed. Attaching '%s' to bone '%s'."), *PreviewActor->ActorID.ToString(), *ConfirmedRecord.BoneName.ToString());

            PreviewActor->AttachToComponent(
                Mesh,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                ConfirmedRecord.BoneName); 
            PreviewActor->SetActorEnableCollision(true);
            
            GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
            PreviewActor = nullptr;
            
            UE_LOG(LogCustomizingPlugin, Log, TEXT("Attachment finalized successfully. State is now Idle."));
        }
        else
        {
            UE_LOG(LogCustomizingPlugin, Error, TEXT("Client_ConfirmAddAttachment: Could not get character mesh. Canceling attachment process."));
            CancelPreview();
        }
    }
    else
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("Server rejected the attachment request. Canceling preview for ActorID: %s"), *ConfirmedRecord.ActorID.ToString());
        CancelPreview(); 
    }
}

void UAttachmentPreviewComponent::Server_SaveAttachmentRecord_Implementation(const FAttachmentRecord& Record, FName PlayerID)
{
    const bool bSuccess = UAttachmentDataStore::Get()->AddAttachment(PlayerID, Record);
    if (bSuccess){
        UE_LOG(LogCustomizingPlugin, Log, TEXT("[Server] Data store update successful. Sending confirmation to client."));
    }
    else
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("[Server] Failed to update data store. Sending failure confirmation to client."));
    }
    Client_ConfirmAddAttachment(bSuccess, Record);
}

bool UAttachmentPreviewComponent::Server_SaveAttachmentRecord_Validate(const FAttachmentRecord& Record, FName PlayerID)
{
    return Record.ActorID != NAME_None && Record.BoneName != NAME_None;
}


void UAttachmentPreviewComponent::LoadExistingAttachments(
    FName LocalID,
    UDataTable* ActorDataTable,
    UObject* OwningActor)
{
    if (!GetOrCacheMesh() || !ActorDataTable || !OwningActor)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("LoadExistingAttachments: invalid parameters"));
        return;
    }

    const auto& Recs = UAttachmentDataStore::Get()->GetAttachments(LocalID);
    for (const FAttachmentRecord& R : Recs)
    {
        AAttachableActor::SpawnAttachment(
            R,
            GetOrCacheMesh(),
            ActorDataTable,
            OwningActor);
    }
}