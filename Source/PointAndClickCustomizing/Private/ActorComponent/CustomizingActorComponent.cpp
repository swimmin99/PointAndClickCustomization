// Copyright 2025 Devhanghae All Rights Reserved.
#include "ActorComponent/CustomizingActorComponent.h"
#include "ActorComponent/AttachmentPreviewComponent.h"
#include "ActorComponent/AttachmentFocusComponent.h"
#include "ActorComponent/StateMachineComponent.h"
#include "Character/CustomCharacter.h"
#include "GameFramework/Actor.h"

UCustomizingActorComponent::UCustomizingActorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    PreviewComp     = CreateDefaultSubobject<UAttachmentPreviewComponent>(TEXT("PreviewComp"));
    FocusComp       = CreateDefaultSubobject<UAttachmentFocusComponent>(TEXT("FocusComp"));

    StateMachine    = CreateDefaultSubobject<UStateMachineComponent>(TEXT("StateMachine"));
}


void UCustomizingActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

void UCustomizingActorComponent::SetCurrentRecord(const FAttachmentRecord& Record)
{
    FocusComp->SetCurrentRecord(Record);
}

const FAttachmentRecord& UCustomizingActorComponent::GetCurrentRecord() const
{
    return FocusComp->GetCurrentRecord();
}

bool UCustomizingActorComponent::RequestSpawnByID(FName ActorID)
{
    return PreviewComp->RequestSpawnByID(ActorID);
}

bool UCustomizingActorComponent::GetAttachmentRecordDataFromGateway(FAttachmentRecord& Rec)
{
    return PreviewComp->GetRecordDataForCurrentActor(IsOwnerLocal(), Rec);
}

void UCustomizingActorComponent::FinalizeAttachmentByAttachingInClient(bool isLocal)
{
    PreviewComp->ApplyAttachmentInClient(IsOwnerLocal());
}


void UCustomizingActorComponent::CancelPreview()
{
    PreviewComp->CancelPreview();
}

AAttachableActor* UCustomizingActorComponent::GetFocusedActor()
{
    return FocusComp->GetFocusedActor();
}

bool UCustomizingActorComponent::TryFocusAttachedActor()
{
    return FocusComp->TryFocusAttachedActor();
}

void UCustomizingActorComponent::CancelFocus()
{
    FocusComp->CancelFocus();
}

void UCustomizingActorComponent::DeleteFocusedActor(FName LocalID)
{
    FocusComp->DeleteFocusedActor(LocalID);
}

void UCustomizingActorComponent::RotateFocusedActor(
    const FVector2D& PrevScreen,
    const FVector2D& CurrScreen,
    const FVector2D& ViewSize,
    float Speed)
{
    FocusComp->RotateFocusedActor(PrevScreen, CurrScreen, ViewSize, Speed);
}

void UCustomizingActorComponent::TrySaveRotation()
{
    FocusComp->EndRotate();
}

ECustomizingState UCustomizingActorComponent::GetState() const
{
    return StateMachine->GetState();
}

void UCustomizingActorComponent::LoadExistingAttachments(FName LocalID)
{
  
       PreviewComp->LoadExistingAttachments(
       LocalID,
       ActorDataTable,
       this);
 }


bool UCustomizingActorComponent::IsOwnerLocal() const
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
