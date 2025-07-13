// WeaponMapComponent.cpp

#include "ActorComponent/WeaponMapComponent.h"
#include "Net/UnrealNetwork.h"

UWeaponMapComponent::UWeaponMapComponent()
{
    SetIsReplicatedByDefault(true);
}

void UWeaponMapComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UWeaponMapComponent, ReplicatedWeaponList);
    DOREPLIFETIME(UWeaponMapComponent, CurrentWeaponID);
}

void UWeaponMapComponent::OnRep_WeaponList()
{
    WeaponMap.Empty();
    
    for (const FWeaponMapEntry& Entry : ReplicatedWeaponList)
    {
        WeaponMap.Add(Entry.WeaponID, Entry.WeaponArray);
    }
    
    UE_LOG(LogTemp, Log, TEXT("[%s] Client's WeaponMap reconstructed from replicated list. Total types: %d"),
        *GetOwner()->GetName(), WeaponMap.Num());
}

void UWeaponMapComponent::OnRep_CurrentWeaponID()
{
    UE_LOG(LogTemp, Log, TEXT("[%s] Client's CurrentWeaponID updated to: %s"),
        *GetOwner()->GetName(), *CurrentWeaponID.ToString());
}

void UWeaponMapComponent::AddWeapon(AShootableActor* TargetActor)
{
    if (!GetOwner()->HasAuthority()) return;
    if (TargetActor->ActorID.IsNone()) return;

    FWeaponArray& WeaponArray = WeaponMap.FindOrAdd(TargetActor->ActorID);
    WeaponArray.Weapons.Add(TargetActor);

    if (CurrentWeaponID == NAME_None)
    {
        CurrentWeaponID = TargetActor->ActorID;
    }
    
    UpdateReplicatedList();
}

void UWeaponMapComponent::UpdateReplicatedList()
{
    if (!GetOwner()->HasAuthority()) return;

    ReplicatedWeaponList.Empty();
    for (const auto& Elem : WeaponMap)
    {
        FWeaponMapEntry NewEntry;
        NewEntry.WeaponID = Elem.Key;
        NewEntry.WeaponArray = Elem.Value;
        ReplicatedWeaponList.Add(NewEntry);
    }
}

void UWeaponMapComponent::Server_FocusNextWeapon_Implementation()
{
    if (WeaponMap.Num() < 2) return;

    TArray<FName> WeaponIDs;
    WeaponMap.GetKeys(WeaponIDs);
    WeaponIDs.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); }); 

    const int32 CurrentIndex = WeaponIDs.Find(CurrentWeaponID);
    const int32 NextIndex = (CurrentIndex + 1) % WeaponIDs.Num();
    CurrentWeaponID = WeaponIDs[NextIndex];
}

void UWeaponMapComponent::Server_FocusPreviousWeapon_Implementation()
{
    if (WeaponMap.Num() < 2) return;
    
    TArray<FName> WeaponIDs;
    WeaponMap.GetKeys(WeaponIDs);
    WeaponIDs.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); });
    const int32 CurrentIndex = WeaponIDs.Find(CurrentWeaponID);
    const int32 PrevIndex = (CurrentIndex - 1 + WeaponIDs.Num()) % WeaponIDs.Num();
    CurrentWeaponID = WeaponIDs[PrevIndex];
}

AShootableActor* UWeaponMapComponent::GetCurrentWeapon() const
{
    if (!IsValid())
    {
        return nullptr;
    }

    const FWeaponArray* FoundArray = WeaponMap.Find(CurrentWeaponID);

    if (FoundArray && FoundArray->Weapons.Num() > 0)
    {
        return FoundArray->Weapons[0].Get();
    }

    return nullptr;
}

TArray<TObjectPtr<AShootableActor>> UWeaponMapComponent::GetCurrentWeapons() const
{
    if (IsValid())
    {
        if (const FWeaponArray* FoundArray = WeaponMap.Find(CurrentWeaponID))
        {
            return FoundArray->Weapons;
        }
    }

    return TArray<TObjectPtr<AShootableActor>>();
}

bool UWeaponMapComponent::IsValid() const
{
    return !CurrentWeaponID.IsNone() && WeaponMap.Contains(CurrentWeaponID);
}