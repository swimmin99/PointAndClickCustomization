// Copyright 2025 Devhanghae All Rights Reserved.
#include "ActorComponent/AttachmentRotationComponent.h"

#include "Actor/AttachableActor.h"
#include "ActorComponent/CustomizingGatewayComponent.h"
#include "ActorComponent/StateMachineComponent.h"
#include "Data/AttachmentDataStore.h"
#include "PlayerController/CustomizingPlayerController.h"
#include "Types/ECustomizingState.h"

UAttachmentRotationComponent::UAttachmentRotationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}


void UAttachmentRotationComponent::HandleRotation(
    const FVector2D& PrevScreen,
    const FVector2D& CurrScreen,
    const FVector2D& ViewSize,
    float Speed,
    AAttachableActor* TargetActor)
{
    FVector v0   = ScreenToArcball(PrevScreen, ViewSize);
    FVector v1   = ScreenToArcball(CurrScreen, ViewSize);
    FVector Axis = FVector::CrossProduct(v0, v1);
    float   Ang  = FMath::Acos(FMath::Clamp(FVector::DotProduct(v0, v1), -1.f, 1.f)) * Speed;

    if (Axis.SizeSquared() > KINDA_SMALL_NUMBER)
    {
        FQuat Delta(Axis.GetSafeNormal(), Ang);
        FQuat Current = TargetActor->GetActorQuat();
        FQuat NewQuat = Current * Delta;  
        TargetActor->SetActorRotation(NewQuat);
    }
}


void UAttachmentRotationComponent::Client_ConfirmUpdateRotation_Implementation(bool bSuccess, FName ActorID, FName BoneID, const FRotator& ConfirmedRotation)
{
    if (bSuccess)
    {
        UE_LOG(LogCustomizingPlugin, Log, TEXT("[Client] Rotation for %s was successfully saved on server."), *ActorID.ToString());
        return; 
    }

    UE_LOG(LogCustomizingPlugin, Warning, TEXT("[Client] Server rejected rotation update for ActorID: %s. Attempting to roll back."), *ActorID.ToString());

    if (ActorID != CurrentRotatingActorID || BoneID != CurrentRotatingBoneName)
    {
        UE_LOG(LogCustomizingPlugin, Error, TEXT("[Client] Mismatched actor response for rotation rollback. Expected '%s', got '%s'."), *CurrentRotatingActorID.ToString(), *ActorID.ToString());
        return;
    }

    if (auto* Gateway = GetOwner()->FindComponentByClass<UCustomizingGatewayComponent>())
    {
        if (AAttachableActor* FocusedActor = Gateway->GetFocusedActor())
        {
            if (FocusedActor->ActorID == ActorID)
            {
                UE_LOG(LogCustomizingPlugin, Log, TEXT("[Client] Reverting rotation of %s to original value: %s"), *ActorID.ToString(), *OriginalRotation.ToString());
                FocusedActor->SetActorRotation(OriginalRotation);
            }
            else
            {
                UE_LOG(LogCustomizingPlugin, Warning, TEXT("[Client] Focused actor '%s' does not match rollback target '%s'."), *FocusedActor->ActorID.ToString(), *ActorID.ToString());
            }
        }
        else
        {
            UE_LOG(LogCustomizingPlugin, Warning, TEXT("[Client] Could not get focused actor to perform rollback."));
        }
    }
}

void UAttachmentRotationComponent::BeginRotation(AAttachableActor* TargetActor)
{
    if (TargetActor)
    {
        OriginalRotation = TargetActor->GetActorRotation();
        CurrentRotatingActorID = TargetActor->ActorID;
        CurrentRotatingBoneName = TargetActor->BoneName;
        UE_LOG(LogCustomizingPlugin, Log, TEXT("BeginRotation: Stored original rotation %s for %s"), *OriginalRotation.ToString(), *TargetActor->ActorID.ToString());
    }
}

void UAttachmentRotationComponent::EndAndSaveRotation(AAttachableActor* TargetActor, FName PlayerID)
{
    if (AAttachableActor* Actor = TargetActor)
    {
        if (auto* Gateway = GetOwner()->FindComponentByClass<UCustomizingGatewayComponent>())
        {
            Server_UpdateRotationData(
                Actor->ActorID,
                Actor->BoneName,
                Actor->GetActorRotation(),
                PlayerID
            );
        }
    }
}

void UAttachmentRotationComponent::Server_UpdateRotationData_Implementation(FName ActorID, FName BoneID,
    const FRotator& NewRotation, FName PlayerID)
{
    bool bSuccess = false; 
    if (auto* Store = UAttachmentDataStore::Get())
    {
            TArray<FAttachmentRecord>& Records = Store->DataMap.FindOrAdd(PlayerID);
            for (FAttachmentRecord& Rec : Records)
            {
                if (Rec.ActorID == ActorID && Rec.BoneName == BoneID)
                {
                    Rec.ActorRotation = NewRotation;
                    UE_LOG(LogCustomizingPlugin, Log,
                        TEXT("%s::UpdateAttachmentRotation - PlayerID=%s, ActorID=%s, BoneID=%s rotation renewd -> %s"),
                        *GetName(),
                        *BoneID.ToString(),
                        *ActorID.ToString(),
                        *BoneID.ToString(),
                        *NewRotation.ToString()
                    );
                    bSuccess = true;
                    break;
                }
            }
        
            Client_ConfirmUpdateRotation(bSuccess, ActorID, BoneID, NewRotation);
        }
}



bool UAttachmentRotationComponent::Server_UpdateRotationData_Validate(FName ActorID, FName BoneID,
    const FRotator& NewRotation, FName PlayerID)
{
    return ActorID != NAME_None && BoneID != NAME_None;
}

FVector UAttachmentRotationComponent::ScreenToArcball(
    const FVector2D& ScreenPos,
    const FVector2D& ViewSize) const
{
    float x  = (2.f * ScreenPos.X / ViewSize.X) - 1.f;
    float y  = 1.f - (2.f * ScreenPos.Y / ViewSize.Y);
    float z2 = 1.f - x*x - y*y;
    float z  = z2 > 0.f ? FMath::Sqrt(z2) : 0.f;
    return FVector(x, y, z).GetSafeNormal();
}
