// Copyright 2025 Devhanghae All Rights Reserved.

#include "UI/AttachmentButtonWidget.h"
#include "PlayerController/CustomizingPlayerController.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

DEFINE_LOG_CATEGORY_STATIC(LogAttachmentButton, Log, All);

void UAttachmentButtonWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogAttachmentButton, Log, TEXT("NativeConstruct called for %s."), *GetName());

    if (AttachmentButton)
    {
        UE_LOG(LogAttachmentButton, Log, TEXT("AttachmentButton is BOUND for %s."), *GetName());
        AttachmentButton->OnClicked.AddDynamic(this, &UAttachmentButtonWidget::OnAttachmentButtonClicked);
        UE_LOG(LogAttachmentButton, Log, TEXT("OnClicked event bound for %s."), *GetName());
    }
    else
    {
        UE_LOG(LogAttachmentButton, Warning, TEXT("AttachmentButton is NOT BOUND for %s! Button clicks will not work."), *GetName());
    }

    if (!AttachmentNameLabel)
    {
        UE_LOG(LogAttachmentButton, Warning, TEXT("AttachmentNameLabel is NOT BOUND for %s."), *GetName());
    }
    else {
        UE_LOG(LogAttachmentButton, Log, TEXT("AttachmentNameLabel is BOUND for %s."), *GetName());
    }
    if (!TypeLabel)
    {
        UE_LOG(LogAttachmentButton, Warning, TEXT("TypeLabel is NOT BOUND for %s."), *GetName());
    }
    else {
        UE_LOG(LogAttachmentButton, Log, TEXT("TypeLabel is BOUND for %s."), *GetName());
    }
    if (!StatLabel) {
        UE_LOG(LogAttachmentButton, Warning, TEXT("StatLabel is NOT BOUND for %s."), *GetName());
    }
    else {
        UE_LOG(LogAttachmentButton, Log, TEXT("StatLabel is BOUND for %s."), *GetName());
    }
    UE_LOG(LogAttachmentButton, Log, TEXT("Initial ActorID for %s: %s"), *GetName(), *ActorID.ToString());
}

void UAttachmentButtonWidget::SetButtonInfo(FName InActorID, const FText& InNameText, const FText& InTypeText, const FText& InStatText)
{
    UE_LOG(LogAttachmentButton, Log, TEXT("SetButtonInfo called for %s. ActorID: %s, Name: '%s', Type: '%s', Stat: '%s'"),
        *GetName(),
        *InActorID.ToString(),
        *InNameText.ToString(),
        *InTypeText.ToString(),
        *InStatText.ToString()
    );

    ActorID = InActorID;

    if (AttachmentNameLabel)
    {
        AttachmentNameLabel->SetText(InNameText);
        UE_LOG(LogAttachmentButton, Log, TEXT("Set AttachmentNameLabel text to: '%s' for %s."), *InNameText.ToString(), *GetName());
    }
    else
    {
        UE_LOG(LogAttachmentButton, Warning, TEXT("AttachmentNameLabel is NOT BOUND for %s; cannot set text."), *GetName());
    }

    if (TypeLabel)
    {
        TypeLabel->SetText(InTypeText);
        UE_LOG(LogAttachmentButton, Log, TEXT("Set TypeLabel text to: '%s' for %s."), *InTypeText.ToString(), *GetName());
    }
    else
    {
        UE_LOG(LogAttachmentButton, Warning, TEXT("TypeLabel is NOT BOUND for %s; cannot set text."), *GetName());
    }

    if (StatLabel)
    {
        StatLabel->SetText(InStatText);
        UE_LOG(LogAttachmentButton, Log, TEXT("Set StatLabel text to: '%s' for %s."), *InStatText.ToString(), *GetName());
    }
    else
    {
        UE_LOG(LogAttachmentButton, Warning, TEXT("StatLabel is NOT BOUND for %s; cannot set text."), *GetName());
    }
}

void UAttachmentButtonWidget::OnAttachmentButtonClicked()
{
    UE_LOG(LogAttachmentButton, Log, TEXT("OnAttachmentButtonClicked called for %s. ActorID to spawn: %s"), *GetName(), *ActorID.ToString());

    ACustomizingPlayerController* CustomPC = GetCustomizingPlayerController();
    if (CustomPC)
    {
        UE_LOG(LogAttachmentButton, Log, TEXT("Found CustomizingPlayerController: %s. Calling TrySpawningAttachableActor..."), *CustomPC->GetName());
        CustomPC->TrySpawningAttachableActor(ActorID);
    }
    else
    {
        UE_LOG(LogAttachmentButton, Warning, TEXT("CustomizingPlayerController NOT FOUND for %s! Cannot spawn ActorID: %s"), *GetName(), *ActorID.ToString());
    }
}

ACustomizingPlayerController* UAttachmentButtonWidget::GetCustomizingPlayerController() const
{
    APlayerController* PC = GetOwningPlayer();
    return PC ? Cast<ACustomizingPlayerController>(PC) : nullptr;
}
