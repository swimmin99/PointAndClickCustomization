// Copyright 2025 Devhanghae All Rights Reserved.
#include "UI/SDynamicItemListPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SOverlay.h"
#include "UI/AttachmentButtonWidget.h"

void SDynamicItemListPanel::Construct(const FArguments& InArgs)
{
    PreCreatedWidgets = InArgs._PreCreatedWidgets;
    PanelWidth = InArgs._PanelWidth;
    ItemHeight = InArgs._ItemHeight;
    PanelPadding = InArgs._PanelPadding;
    ContentPadding = InArgs._ContentPadding;
    Spacing = InArgs._Spacing;
    BlurRadius = InArgs._BlurRadius;
    BlurStrength = InArgs._BlurStrength;

    ChildSlot
        [
            SNew(SOverlay)
                + SOverlay::Slot()
                .HAlign(HAlign_Right)
                .VAlign(VAlign_Fill)
                .Padding(PanelPadding)
                [
                    SNew(SBox)
                        .WidthOverride(PanelWidth)
                        [
                            SNew(SBackgroundBlur)
                                .BlurRadius(BlurRadius)
                                .BlurStrength(BlurStrength)
                                [
                                    SNew(SBorder)
                                        .Padding(ContentPadding)
                                        .BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.8f))
                                        .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                                        [
                                            SNew(SVerticalBox)
                                                + SVerticalBox::Slot()
                                                .AutoHeight()
                                                .Padding(0, 0, 0, Spacing)
                                                [
                                                    SNew(STextBlock)
                                                        .Text(FText::FromString(TEXT("Items")))
                                                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
                                                        .ColorAndOpacity(FLinearColor::White)
                                                ]
                                                + SVerticalBox::Slot()
                                                .FillHeight(1.f)
                                                [
                                                    SNew(SScrollBox)
                                                        .Orientation(Orient_Vertical)
                                                        + SScrollBox::Slot()
                                                        [
                                                            SAssignNew(ItemContainerBox, SVerticalBox)
                                                        ]
                                                ]
                                        ]
                                ]
                        ]
                ]
        ];

    RefreshList();
}

void SDynamicItemListPanel::RefreshList()
{
    if (!ItemContainerBox.IsValid())
        return;

    ItemContainerBox->ClearChildren();

    for (auto& WidgetRef : PreCreatedWidgets)
    {
        ItemContainerBox->AddSlot()
            .AutoHeight()
            .Padding(0, 0, 0, Spacing)
            [
                SNew(SBox)
                    .HeightOverride(ItemHeight)
                    [
                        WidgetRef
                    ]
            ];
    }
}