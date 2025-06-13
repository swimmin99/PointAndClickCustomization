// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Data/FAttachmentRecord.h"
#include "UObject/NoExportTypes.h"
#include "AttachmentDataStore.generated.h"

/**
 * Singleton UObject that holds in-memory attachment records per player.
 * Use Get() to access, then AddAttachment / RemoveAttachment / GetAttachments.
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API UAttachmentDataStore : public UObject
{
	GENERATED_BODY()

public:
	/** Return the global instance (created once and rooted). */
	static UAttachmentDataStore* Get();

	/**
	 * Add a new attachment record for this player.
	 * @param PlayerID  Identifier of the player.
	 * @param Rec       The attachment record to add.
	 */
	bool AddAttachment(FName PlayerID, const FAttachmentRecord& Rec);

	/**
	 * Remove all records matching ActorID & BoneName for this player.
	 * @param PlayerID  Identifier of the player.
	 * @param ActorID   The attachment's ActorID to remove.
	 * @param BoneName  The socket/bone name to match.
	 */
	bool RemoveAttachment(FName PlayerID, FName ActorID, FName BoneName);

	/**
	 * Retrieve all stored attachments for this player.
	 * @param PlayerID  Identifier of the player.
	 * @return Array of records (empty if none).
	 */
	const TArray<FAttachmentRecord>& GetAttachments(FName PlayerID) const;

	/**
	 * Utility to extract from an external map (used for client-side sync).
	 * @param PlayerID      Identifier of the player.
	 * @param TargetDataMap External map of PlayerID->records.
	 */
	const TArray<FAttachmentRecord>& GetAttachmentsFromArray(
		FName PlayerID,
		const TMap<FName, TArray<FAttachmentRecord>>& TargetDataMap
	) const;

	/** In-memory map: PlayerID -> list of attachments. */
	TMap<FName, TArray<FAttachmentRecord>> DataMap;
};