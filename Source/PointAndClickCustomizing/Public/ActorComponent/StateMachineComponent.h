
#pragma once

#include "CoreMinimal.h"
#include "PointAndClickCustomizing.h"
#include "Types/ECustomizingState.h"
#include "Components/ActorComponent.h"
#include "StateMachineComponent.generated.h"

/**
 * Simple state machine component for customizing workflow.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Set a new customizing state. */
	void SetState(ECustomizingState NewState);

	/** Get current state. */
	UFUNCTION(BlueprintCallable, Category="Customizing|State")
	ECustomizingState GetState() const;

private:
	/** Holds the current customizing state. */
	ECustomizingState CurrentState = ECustomizingState::Idle;
};