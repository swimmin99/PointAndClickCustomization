// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "MeshPivotComponent.generated.h"


/**
 * Pivot component used to rotate the customizing mesh around its base.
 * Acts as the parent for the visual mesh in the customization scene.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UMeshPivotComponent : public USceneComponent
{
	GENERATED_BODY()
};