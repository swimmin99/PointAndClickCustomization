// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/FAttachmentRecord.h"
#include "CustomizingSubBaseComponent.generated.h"

class UStateMachineComponent;
class ACustomCharacter;
class USkeletalMeshComponent;
class UAttachmentFocusComponent;
class UCustomizingActorComponent;
class UDataTable;
class AAttachableActor;

/**
 * Base class for all customizing sub-components.:
 *  - StateMachine
 *  - CustomCharacter
 *  - CustomizingMesh
 *  - AttachmentFocusComponent → FocusedActor
 *  - ActorDataTable
 *  - CurrentRecord (for all sub-components)
 *
 *  To support Multiplayer environment caching should be done Asynchronously.
 */
UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UCustomizingSubBaseComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCustomizingSubBaseComponent();

protected:
    UStateMachineComponent*    GetOrCacheStateMachine();
    ACustomCharacter*          GetOrCacheCharacter();
    USkeletalMeshComponent*    GetOrCacheMesh();

    UAttachmentFocusComponent* GetOrCacheFocusComponent();
    UDataTable*                GetOrCacheActorDataTable();
    AAttachableActor*          GetFocusedActor();
   

    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

    UPROPERTY()
    FAttachmentRecord                        CurrentRecord;
    
private:
    TWeakObjectPtr<UStateMachineComponent>    CachedStateMachine;
    TWeakObjectPtr<ACustomCharacter>          CachedCharacter;
    TWeakObjectPtr<USkeletalMeshComponent>    CachedMesh;
    TWeakObjectPtr<UAttachmentFocusComponent> CachedFocusComp;
    TWeakObjectPtr<UDataTable>                CachedActorTable;
};
