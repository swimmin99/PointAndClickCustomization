// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PopupWiddgetBase.generated.h"

struct FPopupData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopupConfirmed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopUpCanceled);

UCLASS()
class POINTANDCLICKCUSTOMIZING_API UPopupWiddgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializePopUp(const FPopupData& InData);

	UPROPERTY(BlueprintAssignable, Category = "PopupEvnets")
	FOnPopupConfirmed OnConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "PopupEvnets")
	FOnPopUpCanceled OnCancelled;


	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> DescriptionText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> PopupImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> ConfirmButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> CancelButton;
	
	UFUNCTION()
	void OnConfirmButtonClicked();

	UFUNCTION()
	void OnCancelButtonClicked();
	
	
};
