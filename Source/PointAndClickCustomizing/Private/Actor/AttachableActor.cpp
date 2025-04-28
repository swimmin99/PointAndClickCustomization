#include "Actor/AttachableActor.h"
#include "PointAndClickCustomizing.h"          
#include "Data/FActorDataRow.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

AAttachableActor::AAttachableActor()
{
}

void AAttachableActor::BeginPlay()
{
    Super::BeginPlay();
    SetReplicates(true);
}

AAttachableActor* AAttachableActor::SpawnAttachment(
    const FAttachmentRecord& Record,
    USkeletalMeshComponent* Skel,
    UDataTable* ActorDataTable,
    UObject* WorldContext)
{
    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("SpawnAttachment - Begin for ActorID=%s Bone=%s"),
        *Record.ActorID.ToString(), *Record.BoneName.ToString());

    if (!Skel || !ActorDataTable || !WorldContext)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnAttachment - Invalid parameters"));
        return nullptr;
    }

    UWorld* World = WorldContext->GetWorld();
    if (!World)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnAttachment - World is null"));
        return nullptr;
    }

    // Find DataTable row
    const FActorDataRow* Row = ActorDataTable->FindRow<FActorDataRow>(
        Record.ActorID, TEXT("SpawnAttachment"));
    if (!Row)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnAttachment - No DataTable row for %s"),
            *Record.ActorID.ToString());
        return nullptr;
    }

    // Spawn the actor
    AActor* Base = World->SpawnActor<AActor>(Row->BlueprintClass);
    if (!Base)
    {
        UE_LOG(LogCustomizingPlugin, Error,
            TEXT("SpawnAttachment - SpawnActor failed for %s"),
            *Record.ActorID.ToString());
        return nullptr;
    }

    AAttachableActor* Att = Cast<AAttachableActor>(Base);
    if (!Att)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnAttachment - Spawned actor is not AAttachableActor, destroying"));
        Base->Destroy();
        return nullptr;
    }

    // Initialize IDs
    Att->ActorID = Record.ActorID;
    Att->BoneName  = Record.BoneName;
    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("SpawnAttachment - Set ActorID=%s BoneID=%s"),
        *Att->ActorID.ToString(), *Att->BoneName.ToString());

    // Attach and restore rotation
    Att->AttachToComponent(
        Skel,
        FAttachmentTransformRules::KeepRelativeTransform, 
        Record.BoneName);
    Att->AddActorWorldRotation(Record.ActorRotation);

#if WITH_EDITOR
    // Visualize attachment point
    const FVector Loc = Att->GetActorLocation();
    DrawDebugSphere(World, Loc, 8.f, 12, FColor::Red, false, 10.f, 0, 1.f);
    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("SpawnAttachment - DebugSphere at %s"), *Loc.ToString());
#endif

    Att->SetActorEnableCollision(true);
    return Att;
}

AAttachableActor* AAttachableActor::SpawnPreview(
    FName InActorID,
    UDataTable* ActorDataTable,
    UObject* WorldContext)
{
    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("SpawnPreview - Begin for ActorID=%s"), *InActorID.ToString());

    if (!ActorDataTable || !WorldContext)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnPreview - Invalid parameters"));
        return nullptr;
    }

    UWorld* World = WorldContext->GetWorld();
    if (!World)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnPreview - World is null"));
        return nullptr;
    }

    // Find DataTable row
    const FActorDataRow* Row = ActorDataTable->FindRow<FActorDataRow>(
        InActorID, TEXT("SpawnPreview"));
    if (!Row)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnPreview - No DataTable row for %s"),
            *InActorID.ToString());
        return nullptr;
    }

    // Spawn the actor
    AActor* Base = World->SpawnActor<AActor>(Row->BlueprintClass);
    if (!Base)
    {
        UE_LOG(LogCustomizingPlugin, Error,
            TEXT("SpawnPreview - SpawnActor failed for %s"), *InActorID.ToString());
        return nullptr;
    }

    AAttachableActor* Att = Cast<AAttachableActor>(Base);
    if (!Att)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnPreview - Spawned actor is not AAttachableActor, destroying"));
        Base->Destroy();
        return nullptr;
    }

    // Initialize ID and disable collision for preview
    Att->ActorID = InActorID;
    Att->SetActorEnableCollision(false);
    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("SpawnPreview - Spawned preview ActorID=%s in Player : %s"), *Att->ActorID.ToString());

    return Att;
}
