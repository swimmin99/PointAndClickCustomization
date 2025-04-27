#pragma once
#include "FCommonStat.h"
#include "FActorDataRow.generated.h"

enum class EActorType : uint8;

/**
 * DataTable row defining an attachable actor's display name, type, class, and stats.
 */
USTRUCT(BlueprintType)
struct FActorDataRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Name shown in UI for this attachable actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Data")
	FName DisplayedName;

	/** Category/type of actor (Weapon, Armor, etc). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Data")
	EActorType Type;

	/** Blueprint class to spawn for this actor. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Data")
	TSubclassOf<AActor> BlueprintClass;

	/** Base stats common to all types. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Data")
	FCommonStat BaseStat;
};

