// Copyright 2025 Devhanghae All Rights Reserved.
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
class POINTANDCLICKCUSTOMIZING_API UAttachmentRotationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttachmentRotationComponent();

	
	void HandleRotation(
		const FVector2D& PrevScreen,
		const FVector2D& CurrScreen,
		const FVector2D& ViewSize,
		float Speed,
		AAttachableActor* TargetActor);
	

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UpdateRotationData(FName ActorID, FName BoneID, const FRotator& NewRotation, FName PlayerID);

	UFUNCTION(Client, Reliable)
	void Client_ConfirmUpdateRotation(bool bSuccess, FName ActorID, FName BoneID, const FRotator& NewRotation);

	void BeginRotation(AAttachableActor* TargetActor);
	void EndAndSaveRotation(AAttachableActor* TargetActor, FName PlayerID);
private:
	FVector ScreenToArcball(const FVector2D& ScreenPos, const FVector2D& ViewSize) const;
	
	FRotator OriginalRotation;
	FName CurrentRotatingActorID;
	FName CurrentRotatingBoneName;
};
