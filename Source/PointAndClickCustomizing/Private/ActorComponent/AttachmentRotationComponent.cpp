// Copyright 2025 Devhanghae All Rights Reserved.
#include "ActorComponent/AttachmentRotationComponent.h"

#include "Actor/AttachableActor.h"
#include "ActorComponent/CustomizingActorComponent.h"
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

void UAttachmentRotationComponent::TrySaveRotation(AAttachableActor* TargetActor)
{
    if (AAttachableActor* Actor = TargetActor)
    {
        if (auto* Gateway = GetOwner()->FindComponentByClass<UCustomizingActorComponent>())
        {
            Server_UpdateRotationData(
                Actor->ActorID,
                Actor->BoneName,
                Actor->GetActorRotation()
            );
        }
    }
}

void UAttachmentRotationComponent::Server_UpdateRotationData_Implementation(FName ActorID, FName BoneID,
    const FRotator& NewRotation)
{
    if (auto* Store = UAttachmentDataStore::Get())
    {
        if (auto* PC = Cast<ACustomizingPlayerController>(GetOwner()))
        {
            TArray<FAttachmentRecord>& Records = Store->DataMap.FindOrAdd(PC->GetPlayerKey());
            for (FAttachmentRecord& Rec : Records)
            {
                if (Rec.ActorID == ActorID && Rec.BoneName == BoneID)
                {
                    Rec.ActorRotation = NewRotation;
                    UE_LOG(LogCustomizingPlugin, Log,
                        TEXT("%s::UpdateAttachmentRotation - PlayerID=%s, ActorID=%s, BoneID=%s rotation renewd -> %s"),
                        *GetName(),
                        *PC->GetPlayerKey().ToString(),
                        *ActorID.ToString(),
                        *BoneID.ToString(),
                        *NewRotation.ToString()
                    );
                    return;
                }
            }

            UE_LOG(LogCustomizingPlugin, Warning,
                TEXT("%s::UpdateAttachmentRotation - No actor found during the search for updating rotation. PlayerID=%s, ActorID=%s, BoneID=%s"),
                *GetName(),
                *PC->GetPlayerKey().ToString(),
                *ActorID.ToString(),
                *BoneID.ToString()
            );
        }
    }
}

bool UAttachmentRotationComponent::Server_UpdateRotationData_Validate(FName ActorID, FName BoneID,
    const FRotator& NewRotation)
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
