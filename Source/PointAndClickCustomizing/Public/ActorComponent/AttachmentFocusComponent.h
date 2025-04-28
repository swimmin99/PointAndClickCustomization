#pragma once

#include "CoreMinimal.h"
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
	AAttachableActor* GetFocusedActor() const { return FocusedActor; }
	bool TryFocusAttachedActor();
	void CancelFocus();
	void DeleteFocusedActor(FName LocalID);
	void SetCurrentRecord(const FAttachmentRecord& InRecord) { CurrentRecord = InRecord; }
	const FAttachmentRecord& GetCurrentRecord() const { return CurrentRecord; }

private:
	UPROPERTY()
	AAttachableActor* CachedCanFocusActor = nullptr;

	UPROPERTY()
	AAttachableActor* FocusedActor = nullptr;
};
