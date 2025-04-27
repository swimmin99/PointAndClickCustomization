#include "Data/AttachmentDataStore.h"

#include "PointAndClickCustomizing.h"

UAttachmentDataStore* UAttachmentDataStore::Get()
{
    static UAttachmentDataStore* Inst = nullptr;
    if (!Inst)
    {
        Inst = NewObject<UAttachmentDataStore>(
            GetTransientPackage(),
            UAttachmentDataStore::StaticClass());
        Inst->AddToRoot();
        UE_LOG(LogCustomizingPlugin, Log, TEXT("AttachmentDataStore::Get - Created singleton instance"));
    }
    return Inst;
}

void UAttachmentDataStore::AddAttachment(FName PlayerID, const FAttachmentRecord& Rec)
{
    // Log and early return if invalid
    if (PlayerID.IsNone())
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("AddAttachment - Invalid PlayerID"));
        return;
    }

    DataMap.FindOrAdd(PlayerID).Add(Rec);
    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("AddAttachment - PlayerID=%s ActorID=%s BoneName=%s"),
        *PlayerID.ToString(), *Rec.ActorID.ToString(), *Rec.BoneName.ToString());
}

void UAttachmentDataStore::RemoveAttachment(
    FName PlayerID,
    FName ActorID,
    FName BoneName)
{
    auto* Arr = DataMap.Find(PlayerID);
    if (!Arr)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("RemoveAttachment - No records for PlayerID=%s"), *PlayerID.ToString());
        return;
    }

    const int32 Before = Arr->Num();
    Arr->RemoveAll([ActorID, BoneName](const FAttachmentRecord& R)
    {
        return (R.ActorID == ActorID && R.BoneName == BoneName);
    });

    const int32 After = Arr->Num();
    UE_LOG(LogCustomizingPlugin, Log,
        TEXT("RemoveAttachment - PlayerID=%s ActorID=%s BoneName=%s Removed=%d"),
        *PlayerID.ToString(), *ActorID.ToString(), *BoneName.ToString(),
        Before - After);
}

const TArray<FAttachmentRecord>& UAttachmentDataStore::GetAttachments(FName PlayerID) const
{
    if (const auto* Arr = DataMap.Find(PlayerID))
    {
        return *Arr;
    }
    static const TArray<FAttachmentRecord> Empty;
    return Empty;
}

const TArray<FAttachmentRecord>& UAttachmentDataStore::GetAttachmentsFromArray(
    FName PlayerID,
    const TMap<FName, TArray<FAttachmentRecord>>& TargetDataMap
) const
{
    if (const auto* Arr = TargetDataMap.Find(PlayerID))
    {
        return *Arr;
    }
    static const TArray<FAttachmentRecord> Empty;
    return Empty;
}