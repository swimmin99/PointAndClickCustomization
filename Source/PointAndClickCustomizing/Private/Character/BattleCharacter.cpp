// Copyright 2025 Devhanghae All Rights Reserved.
#include "Character/BattleCharacter.h"
#include "PointAndClickCustomizing.h"
#include "ActorComponent/AttachmentLoaderComponent.h"

ABattleCharacter::ABattleCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create attachment loader component
	AttachmentLoader = CreateDefaultSubobject<UAttachmentLoaderComponent>(TEXT("AttachmentLoader"));
	UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::Constructor"), *GetName());
}


void ABattleCharacter::SetupPartsForCharacter(FName CallerID)
{
	// Only load on server authority
	if (!HasAuthority() || !AttachmentLoader)
	{
		return;
	}

	// Use mesh component as target
	if (USkeletalMeshComponent* TargetMesh = GetMesh())
	{
		AttachmentLoader->LoadExistingAttachments(TargetMesh);
		UE_LOG(LogCustomizingPlugin, Log,
			TEXT("%s::SetupPartsForCharacter - Loaded attachments for CallerID=%s"),
			*GetName(), *CallerID.ToString());
	}
}


