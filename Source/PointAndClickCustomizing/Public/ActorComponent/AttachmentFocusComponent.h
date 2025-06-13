// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttachmentRotationComponent.h"
#include "ActorComponent/CustomizingSubBaseComponent.h"
#include "Types/ECustomizingState.h"
#include "Data/FAttachmentRecord.h"
#include "AttachmentFocusComponent.generated.h"

class AAttachableActor;
/**
 * Focus Detection & Actor Focus Management
 * Focus is a term for the process of selecting an actor with mouse left click.
 * You can modify the rotation of the attached actor with focused mode
 * You can delete the focused actor with delete key.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UAttachmentFocusComponent : public UCustomizingSubBaseComponent
{
	GENERATED_BODY()

public:
	UAttachmentFocusComponent();

	void UpdateFocusDetection();
	void RotateFocusedActor(const FVector2D& PrevScreen, const FVector2D& CurrScreen, const FVector2D& ViewSize,
	                        float Speed);
	AAttachableActor* GetFocusedActor() const { return FocusedActor; }
	AAttachableActor* GetCachedCanFocusActor() const { return CachedCanFocusActor; }
	void EndRotate(FName PlayerID);
	bool TryFocusAttachedActor();
	void CancelFocus();
	void DeleteFocusedActor(FName PlayerKey);
	void SetCurrentRecord(const FAttachmentRecord& InRecord) { CurrentRecord = InRecord; }
	const FAttachmentRecord& GetCurrentRecord() const { return CurrentRecord; }
	/** Request server to remove the focused attachment. */
	
	UFUNCTION(Server, Reliable, Category="CustomizingPlugin")
	void Server_RequestRemoveAttachment(FName ActorID, FName BoneID, FName PlayerID);
	
	UFUNCTION(Client, Reliable)
	void Client_ConfirmRemoveAttachment(bool bWasSuccessful, FName ActorID, FName BoneID);
	
protected:
	
private:
	UPROPERTY()
	AAttachableActor* CachedCanFocusActor = nullptr;

	UPROPERTY()
	AAttachableActor* FocusedActor = nullptr;

	UPROPERTY()
	UAttachmentRotationComponent*   RotationComp;

	
	float OriginalOrthoWidth = 0.f;
	FVector OriginalSpringWorldLoc = FVector::ZeroVector;
	FVector OriginalSpringRelLoc = FVector::ZeroVector;

	bool bHasStoredCameraState = false;

	UPROPERTY(EditAnywhere, Category="Camera")
	float ZoomOrthoWidth = 150.f;  // 원하는 줌 사이즈

	void ZoomIn();
	void ZoomOut();
};


