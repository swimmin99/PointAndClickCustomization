// Copyright 2025 Devhanghae All Rights Reserved.
#include "ActorComponent/CustomizingGatewayComponent.h"
#include "ActorComponent/AttachmentPreviewComponent.h"
#include "ActorComponent/AttachmentFocusComponent.h"
#include "ActorComponent/StateMachineComponent.h"
#include "Character/CustomCharacter.h"
#include "Data/AttachmentDataStore.h"
#include "GameFramework/Actor.h"

UCustomizingGatewayComponent::UCustomizingGatewayComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    PreviewComp     = CreateDefaultSubobject<UAttachmentPreviewComponent>(TEXT("PreviewComp"));
    FocusComp       = CreateDefaultSubobject<UAttachmentFocusComponent>(TEXT("FocusComp"));
    StateMachine    = CreateDefaultSubobject<UStateMachineComponent>(TEXT("StateMachine"));
}


void UCustomizingGatewayComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;

    if (StateMachine->GetState() == ECustomizingState::Idle)
    {
        FocusComp->UpdateFocusDetection();
        return;
    }

    PreviewComp->Update(DeltaTime);
}

void UCustomizingGatewayComponent::SetCurrentRecord(const FAttachmentRecord& Record)
{
    FocusComp->SetCurrentRecord(Record);
}

const FAttachmentRecord& UCustomizingGatewayComponent::GetCurrentRecord() const
{
    return FocusComp->GetCurrentRecord();
}

bool UCustomizingGatewayComponent::RequestSpawnByID(FName ActorID)
{
    return PreviewComp->RequestSpawnByID(ActorID);
}

void UCustomizingGatewayComponent::FinalizeAttachment(FName PlayerID)
{
    if (IsOwnerLocal())
    {
        PreviewComp->FinalizeAttachment(PlayerID);
    } 
}

void UCustomizingGatewayComponent::CancelPreview()
{
    PreviewComp->CancelPreview();
}

AAttachableActor* UCustomizingGatewayComponent::GetFocusedActor()
{
    return FocusComp->GetFocusedActor();
}

bool UCustomizingGatewayComponent::TryFocusAttachedActor()
{
    return FocusComp->TryFocusAttachedActor();
}

void UCustomizingGatewayComponent::CancelFocus()
{
    FocusComp->CancelFocus();
}

void UCustomizingGatewayComponent::DeleteFocusedActor(FName PlayerID)
{
    FocusComp->DeleteFocusedActor(PlayerID);
}


void UCustomizingGatewayComponent::RotateFocusedActor(
    const FVector2D& PrevScreen,
    const FVector2D& CurrScreen,
    const FVector2D& ViewSize,
    float Speed)
{
    FocusComp->RotateFocusedActor(PrevScreen, CurrScreen, ViewSize, Speed);
}

void UCustomizingGatewayComponent::TrySaveRotation(FName PlayerID)
{
    FocusComp->EndRotate(PlayerID);
}

ECustomizingState UCustomizingGatewayComponent::GetState() const
{
    return StateMachine->GetState();
}

void UCustomizingGatewayComponent::LoadExistingAttachments(FName LocalID)
{
  
       PreviewComp->LoadExistingAttachments(
       LocalID,
       ActorDataTable,
       this);
 }


bool UCustomizingGatewayComponent::IsOwnerLocal() const
{
    if (!bOwnerLocalCacheInitialized)
    {
        if (const auto* PC = Cast<APlayerController>(GetOwner()))
        {
            bIsOwnerLocalCached = PC->IsLocalController();
        }
        bOwnerLocalCacheInitialized = true;
    }
    return bIsOwnerLocalCached;
}
