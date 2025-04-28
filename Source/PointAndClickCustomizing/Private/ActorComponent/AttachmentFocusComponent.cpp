#include "ActorComponent/AttachmentFocusComponent.h"
#include "Actor/AttachableActor.h"
#include "GameFramework/PlayerController.h"
#include "PointAndClickCustomizing.h"
#include "ActorComponent/StateMachineComponent.h"

UAttachmentFocusComponent::UAttachmentFocusComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UAttachmentFocusComponent::UpdateFocusDetection()
{
    auto* PC = Cast<APlayerController>(GetOwner());
    if (!PC)
    {
        GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
        return;
    }

    CachedCanFocusActor = nullptr;
    FHitResult Hit;
    if (PC->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit))
    {
        if (auto* A = Cast<AAttachableActor>(Hit.GetActor()))
        {
            CachedCanFocusActor = A;
            GetOrCacheStateMachine()->SetState(ECustomizingState::ActorCanFocus);
            return;
        }
    }

    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
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

    CachedCanFocusActor = nullptr;
    return true;
}

void UAttachmentFocusComponent::CancelFocus()
{
    FocusedActor = nullptr;
    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
}

void UAttachmentFocusComponent::DeleteFocusedActor(FName LocalID)
{
    if (GetOrCacheStateMachine()->GetState() != ECustomizingState::ActorFocused
        || !FocusedActor)
    {
        return;
    }

    FocusedActor->Destroy();
    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
}
