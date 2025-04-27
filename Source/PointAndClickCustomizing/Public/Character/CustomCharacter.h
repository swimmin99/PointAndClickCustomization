// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interface/CutomizableCharacter.h"
#include "PlayerController/BasePlayerController.h"
#include "CustomCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UArrowComponent;
class UMeshPivotComponent;
class UAttachmentLoaderComponent;

/**
 * Character used in the customization area.
 * Displays model, handles camera, and loads attachments.
 */

UCLASS()
class POINTANDCLICKCUSTOMIZING_API ACustomCharacter : public ACharacter, public ICutomizableCharacter
{
	GENERATED_BODY()
public:
	ACustomCharacter();

	/** Load saved attachments after possession or on demand. */
	virtual void SetupPartsForCharacter(FName CallerID = NAME_None) override;

	/** Ensure components are properly initialized on spawn. */
	virtual void PostInitializeComponents() override;

	/** Return the mesh used for customization. */
	USkeletalMeshComponent* GetCustomizingMesh() const { return CustomizingMesh; }

	/** Direction vector for customization plane. */
	FVector GetCustomizingForwardVector() const
	{
		return CustomizingMesh 
			? CustomizingMesh->GetForwardVector() 
			: FVector::ForwardVector;
	}

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override {}
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override {}

	/** Spring arm for camera positioning. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Camera")
	USpringArmComponent* SpringArmComp;

	/** Camera showing the customization scene. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Camera")
	UCameraComponent* CameraComp;

	/** Pivot to rotate the mesh around its base. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Custom")
	UMeshPivotComponent* MeshPivot;

	/** Skeletal mesh for the character model. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Custom")
	USkeletalMeshComponent* CustomizingMesh;

	/** Debug arrow, only visible in editor. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CustomizingPlugin|Debug")
	UArrowComponent* BottomArrow;

	/** Component that loads attachments onto the mesh. */
	UPROPERTY(VisibleAnywhere, Category="CustomizingPlugin|Attachment")
	UAttachmentLoaderComponent* AttachmentLoader;
};