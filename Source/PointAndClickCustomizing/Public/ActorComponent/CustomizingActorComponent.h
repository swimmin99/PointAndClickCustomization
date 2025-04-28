// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/ECustomizingState.h"
#include "PointAndClickCustomizing.h"          
#include "Actor/AttachableActor.h"
#include "Components/ActorComponent.h"
#include "Data/FAttachmentRecord.h"
#include "CustomizingActorComponent.generated.h"

class ACustomCharacter;
class UStateMachineComponent;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UCustomizingActorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCustomizingActorComponent();

    /** Return or set the record currently being edited. */
    void SetCurrentRecord(const FAttachmentRecord& Record);
    const FAttachmentRecord& GetCurrentRecord() const;
    
    UFUNCTION(BlueprintCallable, Category="Customizing")
    virtual AAttachableActor * GetFocusedActor(){ return Cast<AAttachableActor>(FocusedActor);};
    
    UFUNCTION(BlueprintCallable, Category="Customizing")
    bool RequestSpawnByID(FName ActorID);

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void FinalizeAttachment();

    /** True if this component’s owner is the local client. */
    bool IsOwnerLocal() const;

    UFUNCTION(BlueprintCallable, Category="Customizing")
    ECustomizingState GetState() const;


    UFUNCTION(BlueprintCallable, Category="Customizing")
    bool TryFocusAttachedActor();

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void CancelPreview();

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void CancelFocus();

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void DeleteFocusedActor(FName LocalID);

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void RotateFocusedActor(const FVector2D& DragDelta);

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void TrySaveRotation();

    
    void UpdateAttachmentRotation(FName PlayerID, FName ActorID, FName BoneID, const FRotator& NewRotation);

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void LoadExistingAttachments(FName LocalID);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_SaveAttachmentRecord(const FAttachmentRecord& Record);

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_UpdateRotationData(FName ActorID, FName BoneID, const FRotator& NewRotation);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    // ==== Core references ====
    UPROPERTY()
    UStateMachineComponent* StateMachine = nullptr;

    UPROPERTY()
    ACustomCharacter* MyCharacter = nullptr;

    // ==== Preview & focus ====
    UPROPERTY()
    AActor* PreviewActor = nullptr;

    UPROPERTY()
    AActor* FocusedActor = nullptr;

    UPROPERTY()
    AActor* CachedCanFocusActor = nullptr;

    // ==== Attachment data ====
    UPROPERTY(EditAnywhere, Category="Customizing")
    UDataTable* ActorDataTable = nullptr;

    FAttachmentRecord CurrentRecord;

    // ==== Movement & snapping ====
    UPROPERTY(EditAnywhere, Category="Customizing")
    float SnapThreshold = 25.f;

    UPROPERTY(EditAnywhere, Category="Customizing")
    float MoveInterpSpeed = 10.f;

    FName CurrentSnapBone = NAME_None;
    FVector ClosestBoneLoc = FVector::ZeroVector;
    FVector TargetLocation = FVector::ZeroVector;

    // ==== Helpers ====
    bool TryInitializeCharacter();
    bool TryGetLocalPlayerID(FName& OutPlayerID) const;
    void UpdateCanFocusDetection();
    void UpdateMoving();
    void FixLocation();
    FVector GetMouseIntersectionLoc() const;
    bool IsPreviewNearBone();
    void UpdateDebug() const;

    // ==== Caching ====
    mutable bool bIsOwnerLocalCached = false;
    mutable bool bOwnerLocalCacheInitialized = false;

};
