// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CustomizingSubBaseComponent.h"
#include "Components/ActorComponent.h"
#include "Data/FAttachmentRecord.h"
#include "Types/ECustomizingState.h"
#include "CustomizingActorComponent.generated.h"

class UAttachmentPreviewComponent;
class UAttachmentFocusComponent;
class UAttachmentRotationComponent;
class UStateMachineComponent;

/**
 * Gateway Component for Customizing System
 * Uses sub-components to manage the customizing process.
 * Communicates with the sub-components and PlayerController to handle the customizing process.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UCustomizingActorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCustomizingActorComponent();
    // === Record Getter/Setter ===
    void SetCurrentRecord(const FAttachmentRecord& Record);
    const FAttachmentRecord& GetCurrentRecord() const;

    // === Attach Preview ===
    UFUNCTION(BlueprintCallable, Category="Customizing")
    bool RequestSpawnByID(FName ActorID);

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void FinalizeAttachment();

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void CancelPreview();

    // === Focus ===
    UFUNCTION(BlueprintCallable, Category="Customizing")
    AAttachableActor* GetFocusedActor();

    UFUNCTION(BlueprintCallable, Category="Customizing")
    bool TryFocusAttachedActor();

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void CancelFocus();

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void DeleteFocusedActor(FName LocalID);

    // === Rotation ===
    UFUNCTION(BlueprintCallable, Category="Customizing")
    void RotateFocusedActor(
        const FVector2D& PrevScreen,
        const FVector2D& CurrScreen,
        const FVector2D& ViewSize,
        float Speed);

    UFUNCTION(BlueprintCallable, Category="Customizing")
    void TrySaveRotation();

    // === State Query ===
    UFUNCTION(BlueprintCallable, Category="Customizing")
    ECustomizingState GetState() const;

    // === Persistence / Network ===
    UFUNCTION(BlueprintCallable, Category="Customizing")
    void LoadExistingAttachments(FName LocalID);

    

    UFUNCTION(BlueprintCallable, Category="Customizing")
    UDataTable* GetActorDataTable()
    {
        return ActorDataTable;
    }

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, Category="Customizing")
    UDataTable* ActorDataTable = nullptr;
    
private:
    UPROPERTY()
    UAttachmentPreviewComponent*    PreviewComp;

    UPROPERTY()
    UAttachmentFocusComponent*      FocusComp;
    
    UPROPERTY()
    UStateMachineComponent*         StateMachine;

    bool IsOwnerLocal() const;

    mutable bool bIsOwnerLocalCached = false;
    mutable bool bOwnerLocalCacheInitialized = false;


};
