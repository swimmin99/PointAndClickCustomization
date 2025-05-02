// Copyright 2025 Devhanghae All Rights Reserved.

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
	void SetState(ECustomizingState NewState);

	UFUNCTION(BlueprintCallable, Category="Customizing|State")
	ECustomizingState GetState() const;

private:
	ECustomizingState CurrentState = ECustomizingState::Idle;
};