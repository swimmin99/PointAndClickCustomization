// Copyright 2025 Devhanghae All Rights Reserved.

#include "UI/CustomizingMainHUD.h"
#include "UI/SDynamicItemListPanel.h"
#include "UI/AttachmentButtonWidget.h"
#include "Engine/DataTable.h"
#include "Data/FActorDataRow.h"
#include "GameFramework/PlayerController.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBackgroundBlur.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"


void UCustomizingMainHUD::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCustomizingMainHUD::SetOnReadyButtonClickedDelegate(const FSimpleDelegate& InDelegate, FText InButtonText)
{
    OnReadyButtonClickedDelegate = InDelegate;
    ReadyButtonText = InButtonText;
}

TSharedRef<SWidget> UCustomizingMainHUD::RebuildWidget()
{
    if (IsDesignTime())
    {
        return SNew(SBox)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                    .Text(NSLOCTEXT("MyMainHudWidget", "DesignTimePlaceholder", "Customizing HUD Placeholder"))
            ];
    }

    if (!MyDynamicListPanelWidget.IsValid())
    {
        APlayerController* OwningPC = GetOwningPlayer();
        if (!OwningPC)
        {
            UE_LOG(LogTemp, Error, TEXT("UCustomizingMainHUD::RebuildWidget - OwningPlayerController is null!"));
            // 버튼 강제 상태: 에디터 빌드시 리턴
            return SNullWidget::NullWidget;
        }
        if (!ActorDataTableAsset)
        {
            UE_LOG(LogTemp, Error, TEXT("UCustomizingMainHUD::RebuildWidget - ActorDataTableAsset is not set!"));
            return SNullWidget::NullWidget;
        }
        if (!AttachmentButtonWidgetType)
        {
            UE_LOG(LogTemp, Error, TEXT("UCustomizingMainHUD::RebuildWidget - AttachmentButtonWidgetType is not set!"));
            return SNullWidget::NullWidget;
        }

        TArray<TSharedRef<SWidget>> WidgetRefs;
        for (const FName& RowName : ActorDataTableAsset->GetRowNames())
        {
            FActorDataRow* DataRow = ActorDataTableAsset->FindRow<FActorDataRow>(RowName, TEXT(""));
            if (!DataRow)
                continue;

            UAttachmentButtonWidget* BtnWidget = CreateWidget<UAttachmentButtonWidget>(OwningPC, AttachmentButtonWidgetType);
            if (!BtnWidget)
                continue;

            FText NameText = FText::FromName(DataRow->DisplayedName);
            FText TypeText = StaticEnum<EActorType>()->GetDisplayNameTextByValue((int64)DataRow->Type);
            FText StatText = FText::Format(
                NSLOCTEXT("DynamicList", "StatsFmt", "HP: {0} | ATK: {1} | DEF: {2}"),
                DataRow->BaseStat.HP,
                DataRow->BaseStat.ATK,
                DataRow->BaseStat.DEF
            );
            BtnWidget->SetButtonInfo(RowName, NameText, TypeText, StatText);

            WidgetRefs.Add(BtnWidget->TakeWidget());
        }

        if (WidgetRefs.Num() > 0)
        {
            MyDynamicListPanelWidget = SNew(SDynamicItemListPanel)
                .PreCreatedWidgets(WidgetRefs)
                .PanelWidth(350.f)
                .ItemHeight(100.f)
                .PanelPadding(8.f)
                .ContentPadding(12.f)
                .Spacing(8.f)
                .BlurRadius(10.f)
                .BlurStrength(2.f);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UCustomizingMainHUD::RebuildWidget - WidgetRefs is empty, skipping panel creation."));
        }
    }

  
    return SNew(SOverlay)
        +SOverlay::Slot()
        .HAlign(HAlign_Fill)
        .VAlign(VAlign_Fill)
        [
            SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SBox)
                        .WidthOverride(200.f)
                        [
                            SNew(SBorder)
                                .Padding(8.f)
                                .BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.12f, 0.2f))
                                .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                                [
                                    SNew(SScrollBox)
                                        .Orientation(Orient_Vertical)
                                        + SScrollBox::Slot()
                                        [
                                            SNew(STextBlock)
                                                .AutoWrapText(true)
                                                .Text(UsageText)
                                                .ColorAndOpacity(FLinearColor::White)
                                                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
                                        ]
                                ]
                        ]
                ]

                + SHorizontalBox::Slot()
                .FillWidth(1.f)
                .Padding(8.f, 0.f, 0.f, 0.f)
                [
                    MyDynamicListPanelWidget.IsValid()
                        ? MyDynamicListPanelWidget.ToSharedRef()
                        : SNullWidget::NullWidget
                ]
        ]

        +SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Bottom)
        .Padding(0.f, 0.f, 0.f, 20.f)
        [
            SNew(SBox)
                .WidthOverride(200.f)
                .HeightOverride(50.f)
                [
                    SNew(SButton)
                        .OnClicked_Lambda([this]() -> FReply
                            {
                                if (OnReadyButtonClickedDelegate.IsBound())
                                {
                                    OnReadyButtonClickedDelegate.Execute();
                                }
                                return FReply::Handled();
                            })
                        [
                            SNew(STextBlock)
                                .Text(ReadyButtonText)
                                .Justification(ETextJustify::Center)
                                .ColorAndOpacity(FLinearColor::White)
                                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
                        ]
                ]
        ];
}


void UCustomizingMainHUD::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    MyDynamicListPanelWidget.Reset();
}

void UCustomizingMainHUD::HandleReadyButtonClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Ready Center Button was clicked!"));
}