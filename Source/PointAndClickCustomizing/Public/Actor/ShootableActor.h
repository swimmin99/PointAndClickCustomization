// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actor/AttachableActor.h"
#include "ShootableActor.generated.h"

/**
 * An attachable actor that defines the visual properties of a projectile.
 * This acts as the "gun" or "wand".
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API AShootableActor : public AAttachableActor
{
	GENERATED_BODY()
	
protected:
	// The static mesh to use for projectiles fired from this actor.
	// Set this in the Blueprint for each different "gun".
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TObjectPtr<UStaticMesh> ProjectileMesh;

public:
	// Public getter for the projectile mesh.
	UStaticMesh* GetProjectileMesh() const { return ProjectileMesh; }
};