#pragma once

#include "CoreMinimal.h"


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
