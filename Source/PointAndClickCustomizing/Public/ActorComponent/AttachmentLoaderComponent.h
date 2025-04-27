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
	/** Log all stored attachments for this mesh (editor-only visuals). */
	void DebugLogStoredAttachments(USkeletalMeshComponent* Skel);

	/** Spawn all existing attachments on this mesh. */
	UFUNCTION(BlueprintCallable, Category="Attachment")
	void LoadExistingAttachments(USkeletalMeshComponent* Skel);

	/** Spawn attachments for clients using a provided data map. */
	void LoadExistingAttachmentsForClients(USkeletalMeshComponent* Skel, const TMap<FName, TArray<FAttachmentRecord>>& TargetDataMap);
	
	/** DataTable defining which actor to spawn for each record. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	UDataTable* ActorDataTable;
	
private:
	/** Helper: get the owner’s PlayerID from the mesh. */
	bool TryGetPlayerID(USkeletalMeshComponent* Skel, FName& OutPlayerID) const;

	/** Spawn and attach actor for one record. */
	void SpawnAttachmentFromRecord(const FAttachmentRecord& Record, USkeletalMeshComponent* Skel);

	
};