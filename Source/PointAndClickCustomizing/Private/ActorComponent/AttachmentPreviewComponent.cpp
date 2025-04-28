#include "ActorComponent/AttachmentPreviewComponent.h"
#include "Actor/AttachableActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PointAndClickCustomizing.h"
#include "ActorComponent/StateMachineComponent.h"
#include "Character/CustomCharacter.h"
#include "Data/AttachmentDataStore.h"
#include "PlayerController/CustomizingPlayerController.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

UAttachmentPreviewComponent::UAttachmentPreviewComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UAttachmentPreviewComponent::RequestSpawnByID(FName ActorID)
{
    if (!GetOrCacheCharacter() || !GetOrCacheActorDataTable())
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("RequestSpawnByID failed: Character=%s, Table=%s"),
            *FString(GetOrCacheCharacter() ? TEXT("Valid") : TEXT("Null")),
            *FString(GetOrCacheActorDataTable() ? TEXT("Valid") : TEXT("Null")));
        return false;
    }

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }

    PreviewActor = AAttachableActor::SpawnPreview(ActorID, GetOrCacheActorDataTable(), this);
    if (!PreviewActor)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("SpawnPreview returned null for ID=%s"), *ActorID.ToString());
        return false;
    }


    GetOrCacheStateMachine()->SetState(ECustomizingState::ActorMoving);
    PreviewActor->SetReplicates(false);
    TargetLocation = PreviewActor->GetActorLocation();
    return true;
}

void UAttachmentPreviewComponent::FinalizeAttachment(bool IsLocal)
{
    if (GetOrCacheStateMachine()->GetState() != ECustomizingState::ActorSnapped
        || !PreviewActor)
    {
        return;
    }

    if (auto* Mesh = GetOrCacheMesh())
    {
        PreviewActor->AttachToComponent(
            Mesh,
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            CurrentSnapBone);
        PreviewActor->SetActorEnableCollision(true);
    }

    if(IsLocal)
    {
        FAttachmentRecord Rec;
        Rec.ActorID          = Cast<AAttachableActor>(PreviewActor)->ActorID;
        Rec.BoneName         = CurrentSnapBone;
        FTransform SockT     = GetOrCacheMesh()->GetSocketTransform(CurrentSnapBone, RTS_Component);
        Rec.ActorRotation = (PreviewActor->GetActorQuat() * SockT.GetRotation().Inverse()).Rotator();
        CurrentRecord = Rec;

        AAttachableActor* TargetActor = Cast<AAttachableActor>(PreviewActor);;
        TargetActor->ActorID = Rec.ActorID;
        TargetActor->BoneName = Rec.BoneName;
        
        Server_SaveAttachmentRecord(CurrentRecord);
    }

    
    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
    PreviewActor = nullptr;
}

void UAttachmentPreviewComponent::CancelPreview()
{
    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }
    GetOrCacheStateMachine()->SetState(ECustomizingState::Idle);
}

void UAttachmentPreviewComponent::Update(float DeltaTime)
{
    if (!PreviewActor || !GetOrCacheCharacter())
        return;

    if (IsPreviewNearBone())
        GetOrCacheStateMachine()->SetState(ECustomizingState::ActorSnapped);
    else
        GetOrCacheStateMachine()->SetState(ECustomizingState::ActorMoving);

    if (GetOrCacheStateMachine()->GetState() == ECustomizingState::ActorMoving)
        UpdateMoving();
    else
        FixLocation();

    FVector Current = PreviewActor->GetActorLocation();
    FVector NewLoc  = FMath::VInterpTo(Current, TargetLocation, DeltaTime, MoveInterpSpeed);
    PreviewActor->SetActorLocation(NewLoc);
}

bool UAttachmentPreviewComponent::IsPreviewNearBone()
{
    FVector Loc = GetMouseIntersectionLoc();
    if (auto* Mesh = GetOrCacheMesh())
    {
        CurrentSnapBone = Mesh->FindClosestBone(Loc, &ClosestBoneLoc);
        return FVector::Dist(Loc, ClosestBoneLoc) <= SnapThreshold;
    }
    return false;
}

void UAttachmentPreviewComponent::UpdateMoving()
{
    TargetLocation = GetMouseIntersectionLoc();
}

void UAttachmentPreviewComponent::FixLocation()
{
    TargetLocation = ClosestBoneLoc;
}

FVector UAttachmentPreviewComponent::GetMouseIntersectionLoc()
{
    if (auto* PC = Cast<APlayerController>(GetOwner()))
    {
        FVector2D Screen;
        PC->GetMousePosition(Screen.X, Screen.Y);

        FVector WorldLoc, WorldDir;
        if (UGameplayStatics::DeprojectScreenToWorld(PC, Screen, WorldLoc, WorldDir))
        {
            if (auto* Mesh = const_cast<UAttachmentPreviewComponent*>(this)->GetOrCacheMesh())
            {
                const FVector PlanePoint  = Mesh->GetComponentLocation();
                const FVector PlaneNormal = FVector::CrossProduct(
                    FVector::UpVector,
                    GetOrCacheCharacter()->GetCustomizingForwardVector()
                ).GetSafeNormal();

                return FMath::LinePlaneIntersection(
                    WorldLoc,
                    WorldLoc + WorldDir * 10000.f,
                    PlanePoint,
                    PlaneNormal
                );
            }
        }
    }
    return FVector::ZeroVector;
}

void UAttachmentPreviewComponent::UpdateDebug() const
{
#if WITH_EDITOR
    if (auto* Mesh = const_cast<UAttachmentPreviewComponent*>(this)->GetOrCacheMesh())
    {
        DrawDebugSphere(
            GetWorld(),
            Mesh->GetComponentLocation(),
            25.f, 12,
            FColor::Blue,
            false, 0.1f
        );
    }
#endif
}

void UAttachmentPreviewComponent::Server_SaveAttachmentRecord_Implementation(const FAttachmentRecord& Record)
{
    if (auto* PC = Cast<ACustomizingPlayerController>(GetOwner()))
    {
        UAttachmentDataStore::Get()->AddAttachment(PC->GetPlayerKey(), Record);
    }}

bool UAttachmentPreviewComponent::Server_SaveAttachmentRecord_Validate(const FAttachmentRecord& Record)
{
    return Record.ActorID != NAME_None && Record.BoneName != NAME_None;
}


void UAttachmentPreviewComponent::LoadExistingAttachments(
    FName LocalID,
    UDataTable* ActorDataTable,
    UObject* OwningActor)
{
    if (!GetOrCacheMesh() || !ActorDataTable || !OwningActor)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("LoadExistingAttachments: invalid parameters"));
        return;
    }

    const auto& Recs = UAttachmentDataStore::Get()->GetAttachments(LocalID);
    for (const FAttachmentRecord& R : Recs)
    {
        AAttachableActor::SpawnAttachment(
            R,
            GetOrCacheMesh(),
            ActorDataTable,
            OwningActor);
    }
}