// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "ECustomizingState.generated.h"

/**
 * Represents the current interaction state in the customization workflow.
 */
UENUM(BlueprintType, meta=(DisplayName="Customization State"))
enum class ECustomizingState : uint8
{
	Idle,           // No current action
	ActorMoving,    // Preview actor is being moved
	ActorSnapped,   // Preview actor is snapped to a bone
	ActorFocused,   // An attached actor is selected/focused
	ActorCanFocus   // Hovering over an actor that can be focused
};