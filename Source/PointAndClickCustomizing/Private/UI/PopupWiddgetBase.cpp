// Copyright 2025 Devhanghae All Rights Reserved.


#include "UI/PopupWiddgetBase.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/FPopupData.h"

void UPopupWiddgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UPopupWiddgetBase::OnConfirmButtonClicked);
	}
	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UPopupWiddgetBase::OnCancelButtonClicked);
	}
}

void UPopupWiddgetBase::InitializePopUp(const FPopupData& InData)
{
	if (TitleText)
	{
		TitleText->SetText(InData.Title);
	}
	if (DescriptionText)
	{
		DescriptionText->SetText(InData.Description);
	}
	if (PopupImage && InData.Image.IsValid())
	{
		PopupImage->SetBrushFromTexture(InData.Image.LoadSynchronous());
		PopupImage->SetVisibility(ESlateVisibility::Visible);
	} else if(PopupImage)
	{
		PopupImage->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ConfirmButton && CancelButton)
	{
		switch (InData.ButtonType)
		{
		case EPopupButtonType::ConfirmOnly:
			ConfirmButton->SetVisibility(ESlateVisibility::Visible);
			CancelButton->SetVisibility(ESlateVisibility::Collapsed);
			break;

		case EPopupButtonType::CancelOnly:
			ConfirmButton->SetVisibility(ESlateVisibility::Collapsed);
			CancelButton->SetVisibility(ESlateVisibility::Visible);
			break;

		case EPopupButtonType::ConfirmAndCancel:
			ConfirmButton->SetVisibility(ESlateVisibility::Visible);
			CancelButton->SetVisibility(ESlateVisibility::Visible);
			break;

		default:
			ConfirmButton->SetVisibility(ESlateVisibility::Collapsed);
			CancelButton->SetVisibility(ESlateVisibility::Collapsed);
			break;
		}
	}
}

void UPopupWiddgetBase::OnConfirmButtonClicked()
{
	OnConfirmed.Broadcast();
	RemoveFromParent();
}

void UPopupWiddgetBase::OnCancelButtonClicked()
{
	OnCancelled.Broadcast();
	RemoveFromParent();
}