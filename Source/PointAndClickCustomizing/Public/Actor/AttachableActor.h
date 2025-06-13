// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PointAndClickCustomizing.h"          
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Data/FAttachmentRecord.h"
#include "AttachableActor.generated.h"

/**
 * Actor that can be attached to a skeletal mesh based on a saved record.
 * Provides functions to spawn and preview attachments.
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API AAttachableActor : public AActor
{
    GENERATED_BODY()

public:
    AAttachableActor();
	virtual void BeginPlay() override;
    /** Identifier matching a row in the DataTable. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomizingPlugin|Attachable")
    FName ActorID;

    /** Bone/socket name used for attachment. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomizingPlugin|Attachable")
    FName BoneName;

    /**
     * Spawn an attachable actor, attach it to Skel at the recorded bone,
     * and restore its relative rotation.
     * @param TemplateActorInfo            Saved attachment data.
     * @param Skel              Target skeletal mesh component.
     * @param ActorDataTable    DataTable mapping ActorID to Blueprint class.
     * @param WorldContext      Context for getting UWorld.
     * @return The spawned AAttachableActor, or nullptr on failure.
     */
    static AAttachableActor* SpawnAttachment(
        const FAttachmentRecord& TemplateActorInfo,
        USkeletalMeshComponent* Skel,
        UDataTable* ActorDataTable,
        UObject* WorldContext
    );

    /**
     * Spawn a preview actor (without auto-attachment or collision).
     * @param InActorID         Identifier for preview.
     * @param ActorDataTable    DataTable mapping ActorID to Blueprint class.
     * @param WorldContext      Context for getting UWorld.
     * @return The spawned preview actor, or nullptr on failure.
     */
    static AAttachableActor* SpawnPreview(
        FName InActorID,
        UDataTable* ActorDataTable,
        UObject* WorldContext
    );
};
