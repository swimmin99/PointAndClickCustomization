// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/CutomizableCharacter.h"
#include "BattleCharacter.generated.h"

class UAttachmentLoaderComponent;

UCLASS()
class POINTANDCLICKCUSTOMIZING_API ABattleCharacter : public ACharacter, public ICutomizableCharacter
{
	GENERATED_BODY()

public:
	ABattleCharacter();

	/** Load saved attachments for this character on the server. */
	virtual void SetupPartsForCharacter(FName CallerID = NAME_None) override;

protected:
	/** Component responsible for spawning/loading attachments. */
	UPROPERTY(VisibleAnywhere, Category="CustomizingPlugin|Attachment")
	UAttachmentLoaderComponent* AttachmentLoader;
};
