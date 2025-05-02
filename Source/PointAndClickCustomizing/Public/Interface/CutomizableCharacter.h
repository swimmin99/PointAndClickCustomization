// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PointAndClickCustomizing.h"
#include "CutomizableCharacter.generated.h"

/**
 * Interface for characters that can have attachments loaded onto them.
 * Implement this on any ACharacter that needs to restore saved parts.
 */
UINTERFACE(MinimalAPI)
class UCutomizableCharacter : public UInterface
{
	GENERATED_BODY()
};

class POINTANDCLICKCUSTOMIZING_API ICutomizableCharacter
{
	GENERATED_BODY()

public:
	/**
	 * Called to load or reset all saved attachments on this character.
	 * @param CallerID  Identifier of the player requesting setup (optional).
	 */
	virtual void SetupPartsForCharacter(FName CallerID = NAME_None) = 0;
};
