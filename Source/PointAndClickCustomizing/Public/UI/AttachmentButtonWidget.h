// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Data/FCommonStat.h"
#include "AttachmentButtonWidget.generated.h"

class ACustomizingPlayerController;

/**
 * UAttachmentButtonWidget
 * - A Button made for spawning a certain Actor with according ID key.
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API UAttachmentButtonWidget : public UUserWidget
{
    GENERATED_BODY()

    public:
    UPROPERTY(meta = (BindWidget))
    UButton* AttachmentButton;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* AttachmentNameLabel;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TypeLabel;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* StatLabel;

    UPROPERTY(BlueprintReadWrite, Category = "Config", meta = (ExposeOnSpawn = true))
    FName ActorID;

    UFUNCTION(BlueprintCallable, Category = "Attachment Button")
    void SetButtonInfo(FName InActorID, const FText& InNameText, const FText& InTypeText, const FText& InStatText);

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnAttachmentButtonClicked();

private:
    ACustomizingPlayerController* GetCustomizingPlayerController() const;
};
