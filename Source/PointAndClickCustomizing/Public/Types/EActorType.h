// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "EActorType.generated.h"

/**
 * Examples of type categories for attachable actors.
 */

UENUM(BlueprintType)
enum class EActorType : uint8
{
	Weapon    UMETA(DisplayName = "Weapon"),
	Armor     UMETA(DisplayName = "Armor"),
	Mobility  UMETA(DisplayName = "Mobility")
};
