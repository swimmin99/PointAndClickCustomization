// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PointAndClickCustomizing.h"          
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Data/FAttachmentRecord.h"
#include "AttachableActor.generated.h"

/**
 * Actor that can be attached to a skeletal mesh based on a saved record.
 * Provides functions to spawn and preview attachments.
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API AAttachableActor : public AActor
{
	GENERATED_BODY()

public:
	AAttachableActor();
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomizingPlugin|Attachable")
	FName ActorID;

	void DisableCollision();

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CustomizingPlugin|Attachable")
	FName BoneName;


	static AAttachableActor* SpawnAttachment(
		const FAttachmentRecord& TemplateActorInfo,
		USkeletalMeshComponent* Skel,
		UDataTable* ActorDataTable,
		UObject* WorldContext
	);
	
	static AAttachableActor* SpawnPreview(
		FName InActorID,
		UDataTable* ActorDataTable,
		UObject* WorldContext
	);
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_CollisionDisabled)
	bool bIsCollisionDisabled;

	UFUNCTION()
	void OnRep_CollisionDisabled();

	FVector InitialLocation;

};
