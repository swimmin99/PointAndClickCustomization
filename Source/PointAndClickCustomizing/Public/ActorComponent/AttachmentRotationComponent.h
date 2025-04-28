#pragma once

#include "CoreMinimal.h"
#include "ActorComponent/CustomizingSubBaseComponent.h"
#include "Types/ECustomizingState.h"
#include "AttachmentRotationComponent.generated.h"

class AAttachableActor;

/**
 * Actor Rotation & Rotation Modification Save Managing
 * Uses Arcball rotation for rotation.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UAttachmentRotationComponent : public UCustomizingSubBaseComponent
{
	GENERATED_BODY()

public:
	UAttachmentRotationComponent();

	void RotateFocusedActor(
		const FVector2D& PrevScreen,
		const FVector2D& CurrScreen,
		const FVector2D& ViewSize,
		float Speed);

	void TrySaveRotation();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpdateRotationData(FName ActorID, FName BoneID, const FRotator& NewRotation);

private:
	FVector ScreenToArcball(const FVector2D& ScreenPos, const FVector2D& ViewSize) const;
};
