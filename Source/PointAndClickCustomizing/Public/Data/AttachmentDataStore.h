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
	static UAttachmentDataStore* Get();
	bool AddAttachment(FName PlayerID, const FAttachmentRecord& Rec);
	bool RemoveAttachment(FName ActorID, FName BoneName, FName PlayerID);
	
	const TArray<FAttachmentRecord>& GetAttachments(FName PlayerID) const;

	
	const TArray<FAttachmentRecord>& GetAttachmentsFromArray(
		FName PlayerID,
		const TMap<FName, TArray<FAttachmentRecord>>& TargetDataMap
	) const;

	TMap<FName, TArray<FAttachmentRecord>> DataMap;
};