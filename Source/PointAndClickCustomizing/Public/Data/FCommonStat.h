// File: ActorDataTypes.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FCommonStat.generated.h"

/**
 * Example of Basic stats for any attachable actor.
 */
USTRUCT(BlueprintType)
struct POINTANDCLICKCUSTOMIZING_API FCommonStat
{
	GENERATED_BODY()

	/** Health points. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Stats")
	float HP = 0.f;

	/** Attack value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Stats")
	float ATK = 0.f;

	/** Defense value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Stats")
	float DEF = 0.f;
};