// Copyright 2025 Devhanghae All Rights Reserved.
#include "ActorComponent/CustomizingSubBaseComponent.h"
#include "ActorComponent/StateMachineComponent.h"
#include "ActorComponent/AttachmentFocusComponent.h"
#include "ActorComponent/CustomizingGatewayComponent.h"
#include "Character/CustomCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "PointAndClickCustomizing.h"

UCustomizingSubBaseComponent::UCustomizingSubBaseComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

UStateMachineComponent* UCustomizingSubBaseComponent::GetOrCacheStateMachine()
{
    if (!CachedStateMachine.IsValid())
    {
        CachedStateMachine = GetOwner()->FindComponentByClass<UStateMachineComponent>();
        checkf(CachedStateMachine.IsValid(),
               TEXT("%s: StateMachineComponent missing!"), *GetOwner()->GetName());
    }
    return CachedStateMachine.Get();
}

ACustomCharacter* UCustomizingSubBaseComponent::GetOrCacheCharacter()
{
    if (!CachedCharacter.IsValid())
    {
        if (auto* PC = Cast<APlayerController>(GetOwner()))
        {
            CachedCharacter = Cast<ACustomCharacter>(PC->GetPawn());
            if (!CachedCharacter.IsValid())
            {
                UE_LOG(LogCustomizingPlugin, Warning,
                       TEXT("%s: Pawn is not ACustomCharacter"), *GetOwner()->GetName());
            }
        }
    }
    return CachedCharacter.Get();
}

USkeletalMeshComponent* UCustomizingSubBaseComponent::GetOrCacheMesh()
{
    if (!CachedMesh.IsValid() && GetOrCacheCharacter())
    {
        CachedMesh = CachedCharacter->GetCustomizingMesh();
        if (!CachedMesh.IsValid())
        {
            UE_LOG(LogCustomizingPlugin, Warning,
                   TEXT("%s: CustomizingMesh is null"), *CachedCharacter->GetName());
        }
    }
    return CachedMesh.Get();
}


UAttachmentFocusComponent* UCustomizingSubBaseComponent::GetOrCacheFocusComponent()
{
    if (!CachedFocusComp.IsValid())
    {
        CachedFocusComp = GetOwner()->FindComponentByClass<UAttachmentFocusComponent>();
        checkf(CachedFocusComp.IsValid(),
               TEXT("%s: AttachmentFocusComponent missing!"), *GetOwner()->GetName());
    }
    return CachedFocusComp.Get();
}

UDataTable* UCustomizingSubBaseComponent::GetOrCacheActorDataTable()
{
    if (!CachedActorTable.IsValid())
    {
        if (auto* Gate = GetOwner()->FindComponentByClass<UCustomizingGatewayComponent>())
        {
            CachedActorTable = Gate->GetActorDataTable();
            if (!CachedActorTable.IsValid())
            {
                UE_LOG(LogCustomizingPlugin, Warning,
                       TEXT("%s: ActorDataTable is null"), *Gate->GetName());
            }
        }
        else
        {
            UE_LOG(LogCustomizingPlugin, Error,
                   TEXT("%s: CustomizingActorComponent missing!"), *GetOwner()->GetName());
        }
    }
    return CachedActorTable.Get();
}

AAttachableActor* UCustomizingSubBaseComponent::GetFocusedActor()
{
    if (auto* FC = GetOrCacheFocusComponent())
    {
        return FC->GetFocusedActor();
    }
    return nullptr;
}

void UCustomizingSubBaseComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    Super::OnComponentDestroyed(bDestroyingHierarchy);

    CachedStateMachine.Reset();
    CachedCharacter.Reset();
    CachedMesh.Reset();
    CachedFocusComp.Reset();
    CachedActorTable.Reset();
}
