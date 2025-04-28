#include "ActorComponent/CustomizingActorComponent.h"
#include "PointAndClickCustomizing.h"              
#include "ActorComponent/StateMachineComponent.h"
#include "Types/ECustomizingState.h"
#include "Actor/AttachableActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/CustomCharacter.h"
#include "PlayerController/CustomizingPlayerController.h"
#include "Data/AttachmentDataStore.h"
#include "Kismet/GameplayStatics.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

UCustomizingActorComponent::UCustomizingActorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    StateMachine = CreateDefaultSubobject<UStateMachineComponent>(TEXT("StateMachine"));
    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::Constructor - Component created"), *GetName());
}

bool UCustomizingActorComponent::TryInitializeCharacter()
{
    if (MyCharacter) return true;
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
    {
        if (ACustomCharacter* Char = Cast<ACustomCharacter>(PC->GetPawn()))
        {
            MyCharacter = Char;
            UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::TryInitializeCharacter - Found CustomCharacter"), *GetName());
            return true;
        }
    }
    UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::TryInitializeCharacter - No CustomCharacter found"), *GetName());
    return false;
}

bool UCustomizingActorComponent::TryGetLocalPlayerID(FName& OutPlayerID) const
{
    if (const auto* PC = Cast<ACustomizingPlayerController>(GetOwner()))
    {
        OutPlayerID = PC->GetPlayerKey();
        return true;
    }
    UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::TryGetLocalPlayerID - Owner is not CustomizingPlayerController"), *GetName());
    return false;
}


void UCustomizingActorComponent::Server_UpdateRotationData_Implementation(FName ActorID, FName BoneID,
    const FRotator& NewRotation)
{
    
    if (ACustomizingPlayerController* PC = Cast<ACustomizingPlayerController>(GetOwner()))
    {
        FName PlayerID = PC->GetPlayerKey();
        UpdateAttachmentRotation(PlayerID, ActorID, BoneID, NewRotation);
    }
}

bool UCustomizingActorComponent::Server_UpdateRotationData_Validate(FName ActorID, FName BoneID, const FRotator& NewRotation)
{
    return ActorID != NAME_None && BoneID != NAME_None;
}

void UCustomizingActorComponent::BeginPlay()
{
    Super::BeginPlay();
    TryInitializeCharacter();
}

void UCustomizingActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!IsOwnerLocal() || !TryInitializeCharacter())
        return;

#if WITH_EDITOR
    UpdateDebug();
#endif

    const ECustomizingState State = StateMachine->GetState();
    if (State == ECustomizingState::Idle)
    {
        UpdateCanFocusDetection();
        return;
    }

    if (!PreviewActor)
        return;

    if (IsPreviewNearBone())
    {
        StateMachine->SetState(ECustomizingState::ActorSnapped);
    }
    else
    {
        StateMachine->SetState(ECustomizingState::ActorMoving);
    }

    if (StateMachine->GetState() == ECustomizingState::ActorMoving)
    {
        UpdateMoving();
    }
    else if (StateMachine->GetState() == ECustomizingState::ActorSnapped)
    {
        FixLocation();
    }

    const FVector Current = PreviewActor->GetActorLocation();
    const FVector NewLoc = FMath::VInterpTo(Current, TargetLocation, DeltaTime, MoveInterpSpeed);
    PreviewActor->SetActorLocation(NewLoc);
}



bool UCustomizingActorComponent::RequestSpawnByID(FName ActorID)
{
    if (!IsOwnerLocal() || !ActorDataTable)
        return false;

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }

    PreviewActor = AAttachableActor::SpawnPreview(ActorID, ActorDataTable, this);
    if (!PreviewActor)
    {
        UE_LOG(LogCustomizingPlugin, Error, TEXT("%s::RequestSpawnByID - Preview spawn failed for %s"),
               *GetName(), *ActorID.ToString());
        return false;
    }
    PreviewActor->SetReplicates(false);

    TargetLocation = PreviewActor->GetActorLocation();
    StateMachine->SetState(ECustomizingState::ActorMoving);
    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::RequestSpawnByID - PreviewActor spawned for %s"),
           *GetName(), *ActorID.ToString());
    return true;
}

void UCustomizingActorComponent::FinalizeAttachment()
{
    if (StateMachine->GetState() != ECustomizingState::ActorSnapped || !PreviewActor)
        return;
    auto* Skel = MyCharacter->GetCustomizingMesh();
    PreviewActor->AttachToComponent(
        Skel,
        FAttachmentTransformRules::SnapToTargetIncludingScale,
        CurrentSnapBone);
    PreviewActor->SetActorEnableCollision(true);
    FAttachmentRecord Rec;
    Rec.ActorID          = Cast<AAttachableActor>(PreviewActor)->ActorID;
    Rec.BoneName         = CurrentSnapBone;
    FTransform SockT     = Skel->GetSocketTransform(CurrentSnapBone, RTS_Component);
    Rec.RelativeRotation = (PreviewActor->GetActorQuat() * SockT.GetRotation().Inverse()).Rotator();
    
   
    
    if (IsOwnerLocal())
    {
        Server_SaveAttachmentRecord(Rec);
    } else
    {
        PreviewActor->ForEachComponent<UMeshComponent>(false, [](UMeshComponent* MeshComp)
      {
          MeshComp->SetOnlyOwnerSee(true);
          MeshComp->SetOwnerNoSee(false);
          UE_LOG(LogCustomizingPlugin, Log, TEXT("%s owner-only view applied"), *MeshComp->GetName());
      });
    }
    //Setting Up Properties for Spawned Actor
    AAttachableActor* TargetActor = Cast<AAttachableActor>(PreviewActor);;
    TargetActor->ActorID = Rec.ActorID;
    TargetActor->BoneID = Rec.BoneName;
    
    PreviewActor = nullptr;
    StateMachine->SetState(ECustomizingState::Idle);
}

void UCustomizingActorComponent::UpdateCanFocusDetection()
{
  
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
    {
        CachedCanFocusActor = nullptr;
        FHitResult Hit;
        ETraceTypeQuery TraceQ = UEngineTypes::ConvertToTraceType(ECC_Visibility);
        if (PC->GetHitResultUnderCursorByChannel(TraceQ, true, Hit))
        {
            if (AAttachableActor* A = Cast<AAttachableActor>(Hit.GetActor()))
            {
                CachedCanFocusActor = A;
                StateMachine->SetState(ECustomizingState::ActorCanFocus);
                UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::UpdateCanFocusDetection - CanFocusActor=%s"),
                       *GetName(), *A->GetName());
                return;
            }
        }
    }

    StateMachine->SetState(ECustomizingState::Idle);
}

bool UCustomizingActorComponent::TryFocusAttachedActor()
{
    if (StateMachine->GetState() != ECustomizingState::ActorCanFocus || !CachedCanFocusActor || !TryInitializeCharacter())
        return false;

    FocusedActor = CachedCanFocusActor;
    StateMachine->SetState(ECustomizingState::ActorFocused);

    CurrentRecord.ActorID = Cast<AAttachableActor>(FocusedActor)->ActorID;
    CurrentRecord.BoneName = FocusedActor->GetAttachParentSocketName();

    if (USkeletalMeshComponent* Skel = MyCharacter->GetCustomizingMesh())
    {
        const FTransform SockT = Skel->GetSocketTransform(CurrentRecord.BoneName, RTS_Component);
        const FQuat RelQ = FocusedActor->GetActorQuat() * SockT.GetRotation().Inverse();
        CurrentRecord.RelativeRotation = RelQ.Rotator();
    }

    CachedCanFocusActor = nullptr;

    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::TryFocusAttachedActor - Successfully focused on %s"),
           *GetName(), *GetNameSafe(FocusedActor));

    return true;
}

void UCustomizingActorComponent::CancelPreview()
{
    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }
    StateMachine->SetState(ECustomizingState::Idle);
    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::CancelPreview"), *GetName());
}



void UCustomizingActorComponent::CancelFocus()
{
    if (FocusedActor)
    {
        FocusedActor = nullptr;
    }
    StateMachine->SetState(ECustomizingState::Idle);
    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::CancelFocus"), *GetName());
}




void UCustomizingActorComponent::DeleteFocusedActor(FName LocalID)
{
    if (StateMachine->GetState() != ECustomizingState::ActorFocused || !FocusedActor)
        return;
    FName PlayerID = LocalID;

    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::DeleteFocusedActor - Start Removing ActorID=%s for PlayerID=%s"),
             *GetName(), *CurrentRecord.ActorID.ToString(), *PlayerID.ToString());

    
    FocusedActor->Destroy();
    AAttachableActor* TargetAttachableActor = Cast<AAttachableActor>(FocusedActor);
    if (TargetAttachableActor)
        UAttachmentDataStore::Get()->RemoveAttachment(CurrentRecord.ActorID, TargetAttachableActor->ActorID ,TargetAttachableActor->BoneID);
    else
    {
        UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::DeleteFocusedActor - Cannot Cast Target into AttachableActor. Target : %s"), *GetName(), *FocusedActor->GetName())
    }
    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::DeleteFocusedActor - Removed ActorID=%s for PlayerID=%s"),
           *GetName(), *CurrentRecord.ActorID.ToString(), *PlayerID.ToString());

    FocusedActor = nullptr;
    StateMachine->SetState(ECustomizingState::Idle);
}

void UCustomizingActorComponent::RotateFocusedActor(const FVector2D& DragDelta)
{
    if (StateMachine->GetState() != ECustomizingState::ActorFocused
        || !FocusedActor || !TryInitializeCharacter())
    {
        return;
    }

    const FQuat Qyaw(FVector::UpVector, FMath::DegreesToRadians(DragDelta.X * 0.5f));
    const FQuat Qpitch(MyCharacter->GetCustomizingMesh()->GetRightVector(),
                       FMath::DegreesToRadians(-DragDelta.Y * 0.5f));

    FocusedActor->AddActorWorldRotation(Qyaw * Qpitch);

    if (USkeletalMeshComponent* Skel = MyCharacter->GetCustomizingMesh())
    {
        const FTransform ParentT = Skel->GetSocketTransform(CurrentRecord.BoneName, RTS_Component);
        const FQuat RelQ = FocusedActor->GetActorQuat() * ParentT.GetRotation().Inverse();
        CurrentRecord.RelativeRotation = RelQ.Rotator();
    }
    
    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::RotateFocusedActor - Actor rotated"), *GetName());
}

void UCustomizingActorComponent::TrySaveRotation()
{
    if (AAttachableActor* TargetActor = GetFocusedActor())
    {
        Server_UpdateRotationData(TargetActor->ActorID, TargetActor->BoneID, CurrentRecord.RelativeRotation);
        UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::TrySaveRotation - Saved rotation for ActorID=%s, BoneID=%s"),
            *GetName(), *TargetActor->ActorID.ToString(), *TargetActor->BoneID.ToString());
    }
    else
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::TrySaveRotation - No focused actor to save rotation"), *GetName());
    }
}


void UCustomizingActorComponent::UpdateAttachmentRotation(
    FName PlayerID,
    FName ActorID,
    FName BoneID,
    const FRotator& NewRotation
)
{
    if (auto* Store = UAttachmentDataStore::Get())
    {
        TArray<FAttachmentRecord>& Records = Store->DataMap.FindOrAdd(PlayerID);
        for (FAttachmentRecord& Rec : Records)
        {
            if (Rec.ActorID == ActorID && Rec.BoneName == BoneID)
            {
                Rec.RelativeRotation = NewRotation;
                UE_LOG(LogCustomizingPlugin, Log,
                    TEXT("%s::UpdateAttachmentRotation - PlayerID=%s, ActorID=%s, BoneID=%s rotation renewd -> %s"),
                    *GetName(),
                    *PlayerID.ToString(),
                    *ActorID.ToString(),
                    *BoneID.ToString(),
                    *NewRotation.ToString()
                );
                return;
            }
        }

        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("%s::UpdateAttachmentRotation - No actor found during the search for updating rotation. PlayerID=%s, ActorID=%s, BoneID=%s"),
            *GetName(),
            *PlayerID.ToString(),
            *ActorID.ToString(),
            *BoneID.ToString()
        );
    }
}



void UCustomizingActorComponent::LoadExistingAttachments(FName LocalID)
{
    if (!MyCharacter || !ActorDataTable) return;

    
    FName PlayerID = LocalID;
    
    const auto& Recs = UAttachmentDataStore::Get()
        ->GetAttachments(PlayerID);

    for (const FAttachmentRecord& R : Recs)
    {
        AAttachableActor::SpawnAttachment(
            R,
            MyCharacter->GetCustomizingMesh(),
            ActorDataTable,
            this);
    }
}

bool UCustomizingActorComponent::Server_SaveAttachmentRecord_Validate(const FAttachmentRecord& Record)
{
    return Record.ActorID != NAME_None && Record.BoneName != NAME_None;
}

void UCustomizingActorComponent::Server_SaveAttachmentRecord_Implementation(const FAttachmentRecord& Record)
{
    
    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::Server_SaveAttachmentRecord - ActorID=%s"),
           *GetName(), *Record.ActorID.ToString());

    if (ACustomizingPlayerController* PC = Cast<ACustomizingPlayerController>(GetOwner()))
    {
        FName PlayerID = PC->GetPlayerKey();
        UAttachmentDataStore::Get()->AddAttachment(PlayerID, Record);
        UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::Server_SaveAttachmentRecord (ServerRPC) - Saved for PlayerID=%s, BoneID = %s, ActorID = %s"),
               *GetName(), *PlayerID.ToString(), *Record.BoneName.ToString(), *Record.ActorID.ToString());
    }
}

void UCustomizingActorComponent::UpdateMoving()
{
    TargetLocation = GetMouseIntersectionLoc();
}

void UCustomizingActorComponent::FixLocation()
{
    TargetLocation = ClosestBoneLoc;
}

bool UCustomizingActorComponent::IsPreviewNearBone()
{
    if (!PreviewActor || !MyCharacter) return false;

    USkeletalMeshComponent* Skel = MyCharacter->GetCustomizingMesh();
    CurrentSnapBone = Skel->FindClosestBone(GetMouseIntersectionLoc(), &ClosestBoneLoc);

    const float Dist = FVector::Dist(GetMouseIntersectionLoc(), ClosestBoneLoc);
    const bool bSnap = Dist <= SnapThreshold;
    if (bSnap)
    {
        UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::IsPreviewNearBone - SnapBone=%s Dist=%.2f"),
               *GetName(), *CurrentSnapBone.ToString(), Dist);
    }
    return bSnap;
}

FVector UCustomizingActorComponent::GetMouseIntersectionLoc() const
{
    if (!GetWorld())
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::GetMouseIntersectionLoc - World is null"), *GetName());
        return FVector::ZeroVector;
    }
    ABasePlayerController* PC = Cast<ABasePlayerController>(GetOwner());
    if (!PC)
    {
        UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::GetMouseIntersectionLoc - Owner is not ABasePlayerController"), *GetName());
        return FVector::ZeroVector;
    }

    FVector2D ScreenPos;
    bool bGotMouse = PC->GetMousePosition(ScreenPos.X, ScreenPos.Y);
    UE_LOG(LogCustomizingPlugin, Verbose, TEXT("%s::GetMouseIntersectionLoc - MousePos=%s Got=%d"),
           *GetName(), *ScreenPos.ToString(), bGotMouse);

    FVector WorldLoc, WorldDir;
    bool bDeprojected = bGotMouse && UGameplayStatics::DeprojectScreenToWorld(PC, ScreenPos, WorldLoc, WorldDir);
    UE_LOG(LogCustomizingPlugin, Verbose, TEXT("%s::GetMouseIntersectionLoc - Deprojected=%d"),
           *GetName(), bDeprojected);

    if (bDeprojected && MyCharacter)
    {
        const FVector PlanePoint  = MyCharacter->GetCustomizingMesh()->GetComponentLocation();
        const FVector PlaneNormal = FVector::CrossProduct(FVector::UpVector, MyCharacter->GetCustomizingForwardVector()).GetSafeNormal();
        return FMath::LinePlaneIntersection(WorldLoc, WorldLoc + WorldDir * 10000.f, PlanePoint, PlaneNormal);
    }

    UE_LOG(LogCustomizingPlugin, Warning, TEXT("%s::GetMouseIntersectionLoc - Failed deprojection or missing character"), *GetName());
    return FVector::ZeroVector;
}

void UCustomizingActorComponent::UpdateDebug() const
{
#if WITH_EDITOR
    if (!MyCharacter) return;

    const FVector Loc   = MyCharacter->GetCustomizingMesh()->GetComponentLocation();
    const FVector Fwd   = MyCharacter->GetCustomizingForwardVector().GetSafeNormal();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Fwd).GetSafeNormal();
    const FVector Up    = FVector::CrossProduct(Fwd, Right);

    DrawDebugLine(GetWorld(), Loc, Loc + Up * 100.f,    FColor::Red,  false, 0.1f, 0, 4.f);
    DrawDebugLine(GetWorld(), Loc, Loc + Fwd * 100.f,   FColor::Blue, false, 0.1f, 0, 4.f);
    DrawDebugSphere(GetWorld(), Loc, 25.f, 12, FColor::Blue, false, 0.1f, 0, 4.f);
#endif
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


ECustomizingState UCustomizingActorComponent::GetState() const
{
    return StateMachine->GetState();
}

void UCustomizingActorComponent::SetCurrentRecord(const FAttachmentRecord& Record)
{
    CurrentRecord = Record;
}

const FAttachmentRecord& UCustomizingActorComponent::GetCurrentRecord() const
{
    return CurrentRecord;
}
