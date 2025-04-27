#include "ActorComponent/AttachmentLoaderComponent.h"
#include "PointAndClickCustomizing.h"      
#include "Actor/AttachableActor.h"
#include "Data/FAttachmentRecord.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "Data/AttachmentDataStore.h"
#include "DrawDebugHelpers.h"

bool UAttachmentLoaderComponent::TryGetPlayerID(USkeletalMeshComponent* Skel, FName& OutPlayerID) const
{
    if (!Skel)
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::TryGetPlayerID - Invalid SkeletalMeshComponent"), *GetName());
        return false;
    }

    APawn* Pawn = Cast<APawn>(Skel->GetOwner());
    if (!Pawn)
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::TryGetPlayerID - Owner is not a Pawn"), *GetName());
        return false;
    }

    APlayerController* PC = Pawn->GetController<APlayerController>();
    if (!PC || !PC->PlayerState)
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::TryGetPlayerID - Invalid PlayerController or PlayerState"), *GetName());
        return false;
    }

    FUniqueNetIdRepl NetId = PC->PlayerState->GetUniqueId();
    if (!NetId.IsValid())
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::TryGetPlayerID - Invalid UniqueNetId"), *GetName());
        return false;
    }

    OutPlayerID = FName(*NetId->ToString());
    return true;
}

void UAttachmentLoaderComponent::DebugLogStoredAttachments(USkeletalMeshComponent* Skel)
{
    FName PlayerID;
    if (!TryGetPlayerID(Skel, PlayerID))
        return;

    const auto& Recs = UAttachmentDataStore::Get()->GetAttachments(PlayerID);
    int32 Count = 0;

    for (const FAttachmentRecord& R : Recs)
    {
        if (R.BoneName == NAME_None)
            continue;

        const FTransform T = Skel->GetSocketTransform(R.BoneName, ERelativeTransformSpace::RTS_World);
        const FVector Loc = T.GetLocation();

        UE_LOG(LogCustomizingPlugin, Log,
            TEXT("%s::DebugLogStoredAttachments [%d] PlayerID=%s ActorID=%s Bone=%s Loc=%s"),
            *GetName(), ++Count,
            *PlayerID.ToString(),
            *R.ActorID.ToString(),
            *R.BoneName.ToString(),
            *Loc.ToString());

#if WITH_EDITOR
        DrawDebugSphere(GetWorld(), Loc, 10.f, 12, FColor::Green, false, 10.f);
#endif
    }

    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("%s::DebugLogStoredAttachments - Total bones logged: %d"), *GetName(), Count);
}

void UAttachmentLoaderComponent::LoadExistingAttachments(USkeletalMeshComponent* Skel)
{
    FName PlayerID;
    if (!TryGetPlayerID(Skel, PlayerID))
        return;

    if (!ActorDataTable)
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::LoadExistingAttachments - ActorDataTable is null"), *GetName());
        return;
    }

    const auto& Recs = UAttachmentDataStore::Get()->GetAttachments(PlayerID);
    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("%s::LoadExistingAttachments - %d records for %s"),
        *GetName(), Recs.Num(), *PlayerID.ToString());

    for (const FAttachmentRecord& R : Recs)
    {
        SpawnAttachmentFromRecord(R, Skel);
    }

    DebugLogStoredAttachments(Skel);
}

void UAttachmentLoaderComponent::LoadExistingAttachmentsForClients(USkeletalMeshComponent* Skel, const TMap<FName, TArray<FAttachmentRecord>>& TargetDataMap)
{
    FName PlayerID;
    if (!TryGetPlayerID(Skel, PlayerID))
        return;

    if (!ActorDataTable)
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::LoadExistingAttachmentsForClients - ActorDataTable is null"), *GetName());
        return;
    }

    if (const TArray<FAttachmentRecord>* Recs = TargetDataMap.Find(PlayerID))
    {
        UE_LOG(LogCustomizingPlugin, Log,
            TEXT("%s::LoadExistingAttachmentsForClients - %d records for %s"),
            *GetName(), Recs->Num(), *PlayerID.ToString());

        for (const FAttachmentRecord& R : *Recs)
        {
            SpawnAttachmentFromRecord(R, Skel);
        }
    }
    else
    {
        UE_LOG(LogCustomizingPlugin, Log,
            TEXT("%s::LoadExistingAttachmentsForClients - No records for %s"),
            *GetName(), *PlayerID.ToString());
    }
}

void UAttachmentLoaderComponent::SpawnAttachmentFromRecord(const FAttachmentRecord& Record, USkeletalMeshComponent* Skel)
{
    if (!AAttachableActor::SpawnAttachment(Record, Skel, ActorDataTable, this))
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("%s::SpawnAttachmentFromRecord - Failed to spawn ActorID=%s"),
            *GetName(), *Record.ActorID.ToString());
    }
}
