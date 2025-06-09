// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomizingMainHUD.generated.h"

class SDynamicItemListPanel;
class UDataTable;
class UAttachmentButtonWidget;

UCLASS()
class POINTANDCLICKCUSTOMIZING_API UCustomizingMainHUD : public UUserWidget
{
    GENERATED_BODY()

    public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment Panel Config")
    UDataTable* ActorDataTableAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment Panel Config")
    TSubclassOf<UAttachmentButtonWidget> AttachmentButtonWidgetType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment Panel Config")
    FText UsageText = FText::FromString(
        TEXT(
            "HOW TO USE\n"
            "\n"
            "1. Press the GUI button to spawn a part.\n"
            "\n"
            "2. Right-click to cancel spawning.\n"
            "\n"
            "3. Move the spawned part close to a character bone\n"
            "\n"
            "4. Left-click when snapped to attach the part to the character.\n"
            "\n"
            "5. Click an attached part to enter Focus mode.\n"
            "\n"
            "6. In Focus mode, press the Delete key to remove the part, or drag to rotate it.\n"
            "\n"
            "7. Right-click to exit Focus mode.\n"
        )
    );

    void SetOnReadyButtonClickedDelegate(const FSimpleDelegate& InDelegate, FText InButtonText);

protected:
    virtual void NativeConstruct() override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
    TSharedPtr<SDynamicItemListPanel> MyDynamicListPanelWidget;
    FSimpleDelegate OnReadyButtonClickedDelegate;
    FText ReadyButtonText;
    void HandleReadyButtonClicked();
};