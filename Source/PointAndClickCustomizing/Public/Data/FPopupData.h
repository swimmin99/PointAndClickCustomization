// Copyright 2025 Devhanghae All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "FPopupData.generated.h"



UENUM(BlueprintType)
enum class EPopupButtonType : uint8
{
	ConfirmOnly,     
	CancelOnly,      
	ConfirmAndCancel
};

DECLARE_DYNAMIC_DELEGATE(FPopupDelegate);

USTRUCT(BlueprintType)
struct FPopupData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
	TSoftObjectPtr<UTexture2D> Image;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Popup")
	EPopupButtonType ButtonType = EPopupButtonType::ConfirmAndCancel;

	UPROPERTY()
	FPopupDelegate OnConfirmed;

	UPROPERTY()
	FPopupDelegate OnCancelled;
};
