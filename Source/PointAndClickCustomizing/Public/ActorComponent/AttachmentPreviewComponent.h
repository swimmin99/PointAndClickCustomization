// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "ActorComponent/CustomizingSubBaseComponent.h"
#include "Types/ECustomizingState.h"
#include "Data/FAttachmentRecord.h"
#include "AttachmentPreviewComponent.generated.h"

class UDataTable;
class AAttachableActor;

/**
 * Attach Preview, Snap & Movement Management
 * Term "Preview" means the temporary actor that is spawned for previewing the attachment.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UAttachmentPreviewComponent : public UCustomizingSubBaseComponent
{
	GENERATED_BODY()

public:
	UAttachmentPreviewComponent();

	void LoadExistingAttachments(FName LocalID, UDataTable* ActorDataTable, UObject* OwningActor);
	bool RequestSpawnByID(FName ActorID);
	void FinalizeAttachment(bool IsLocal);
	void CancelPreview();
	void Update(float DeltaTime);
	void UpdateDebug();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SaveAttachmentRecord(const FAttachmentRecord& Record);


private:
	UPROPERTY()
	AAttachableActor* PreviewActor = nullptr;
	float SnapThreshold   = 25.f;
	float MoveInterpSpeed = 10.f;
	FName CurrentSnapBone = NAME_None;
	FVector ClosestBoneLoc= FVector::ZeroVector;
	FVector TargetLocation= FVector::ZeroVector;

	// ==== Helper ====
	FVector GetMouseIntersectionLoc();
	bool    IsPreviewNearBone();
	void    UpdateMoving();
	void    FixLocation();
};
