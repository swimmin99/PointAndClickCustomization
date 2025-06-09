// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SVerticalBox;
/**
 * 
 */

struct FDynamicItemInfo
{
    FName    ID;
    FText    Label;
    FText    TypeText;    
    FText    StatText;    

    FDynamicItemInfo(
        const FName& InID,
        const FText& InLabel,
        const FText& InTypeText,
        const FText& InStatText
    )
        : ID(InID)
        , Label(InLabel)
        , TypeText(InTypeText)
        , StatText(InStatText)
    {
    }
};

class POINTANDCLICKCUSTOMIZING_API SDynamicItemListPanel : public SCompoundWidget
{

public:
    SLATE_BEGIN_ARGS(SDynamicItemListPanel)
        : _PreCreatedWidgets()
        , _PanelWidth(400.f)
        , _ItemHeight(60.f)
        , _PanelPadding(10.f)
        , _ContentPadding(20.f)
        , _Spacing(10.f)
        , _BlurRadius(15.f)
        , _BlurStrength(3.f)
        {
        }
        SLATE_ARGUMENT(TArray<TSharedRef<SWidget>>, PreCreatedWidgets)

        SLATE_ARGUMENT(float, PanelWidth)
        SLATE_ARGUMENT(float, ItemHeight)
        SLATE_ARGUMENT(float, PanelPadding)
        SLATE_ARGUMENT(float, ContentPadding)
        SLATE_ARGUMENT(float, Spacing)
        SLATE_ARGUMENT(float, BlurRadius)
        SLATE_ARGUMENT(float, BlurStrength)

    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    void RefreshList();

private:
    TSharedPtr<SVerticalBox> ItemContainerBox;

    TArray<TSharedRef<SWidget>> PreCreatedWidgets;
    float PanelWidth;
    float ItemHeight;
    float PanelPadding;
    float ContentPadding;
    float Spacing;
    float BlurRadius;
    float BlurStrength;
};