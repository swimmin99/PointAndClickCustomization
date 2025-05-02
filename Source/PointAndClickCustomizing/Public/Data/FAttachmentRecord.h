// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FAttachmentRecord.generated.h"



/**
 * Saves which ActorID was attached to which bone and its relative rotation.
 */
USTRUCT(BlueprintType)
struct POINTANDCLICKCUSTOMIZING_API FAttachmentRecord
{
	GENERATED_BODY()

	/** DataTable key for the attachable actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomizingPlugin|Attachment")
	FName ActorID;

	/** Bone/socket name on the skeletal mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomizingPlugin|Attachment")
	FName BoneName;

	/** Local rotation applied after attachment. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomizingPlugin|Attachment")
	FRotator ActorRotation;
};