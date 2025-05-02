// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PointAndClickCustomizing.h" 
#include "Components/ActorComponent.h"
#include "Data/FAttachmentRecord.h"
#include "Engine/DataTable.h"
#include "AttachmentLoaderComponent.generated.h"

/**
 * Component responsible for loading saved attachments onto a skeletal mesh.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UAttachmentLoaderComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void DebugLogStoredAttachments(USkeletalMeshComponent* Skel);

	UFUNCTION(BlueprintCallable, Category="Attachment")
	void LoadExistingAttachments(USkeletalMeshComponent* Skel);

	void LoadExistingAttachmentsForClients(USkeletalMeshComponent* Skel, const TMap<FName, TArray<FAttachmentRecord>>& TargetDataMap);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	UDataTable* ActorDataTable;
	
private:
	bool TryGetPlayerID(USkeletalMeshComponent* Skel, FName& OutPlayerID) const;
	void SpawnAttachmentFromRecord(const FAttachmentRecord& Record, USkeletalMeshComponent* Skel);

	
};