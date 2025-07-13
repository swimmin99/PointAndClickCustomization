// Copyright 2025 Devhanghae All Rights Reserved.


#include "UI/PopupManagerSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "UI/PopupWiddgetBase.h"

void UPopupManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FString Path = TEXT("/PointAndClickCustomizing/PointAndClickCustomizing/Widgets/WBP_PopUpdWidget.WBP_PopUpdWidget_C");
    
    PopupWidgetClass = TSoftClassPtr<UPopupWiddgetBase>(FSoftObjectPath(Path)).LoadSynchronous();
    
    if (!PopupWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PopupWidgetClass is not loaded!"));
    }
}

void UPopupManagerSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UPopupManagerSubsystem::RequestShowPopup(const FPopupData& InData)
{
    PopupQueue.Enqueue(InData);
    ProcessQueue();
}

void UPopupManagerSubsystem::ProcessQueue()
{
    if (CurrentPopupWidget || PopupQueue.IsEmpty() || !PopupWidgetClass)
    {
        return;
    }

    FPopupData Data;
    PopupQueue.Dequeue(Data);

    CurrentConfirmDelegate = Data.OnConfirmed;
    CurrentCancelDelegate = Data.OnCancelled;

    CurrentPopupWidget = CreateWidget<UPopupWiddgetBase>(GetGameInstance(), PopupWidgetClass);
    if (CurrentPopupWidget)
    {
        CurrentPopupWidget->OnConfirmed.AddDynamic(this, &UPopupManagerSubsystem::OnCurrentPopupConfirmed);
        CurrentPopupWidget->OnCancelled.AddDynamic(this, &UPopupManagerSubsystem::OnCurrentPopupCancelled);
        
        CurrentPopupWidget->InitializePopUp(Data);
        CurrentPopupWidget->AddToViewport(100);
    }
}

void UPopupManagerSubsystem::OnCurrentPopupConfirmed()
{
    CurrentConfirmDelegate.ExecuteIfBound();
    CloseCurrentPopupAndProcessNext();
}

void UPopupManagerSubsystem::OnCurrentPopupCancelled()
{
    CurrentCancelDelegate.ExecuteIfBound();
    CloseCurrentPopupAndProcessNext();
}

void UPopupManagerSubsystem::CloseCurrentPopupAndProcessNext()
{
    CurrentPopupWidget = nullptr;
    CurrentConfirmDelegate.Unbind();
    CurrentCancelDelegate.Unbind();
    
    ProcessQueue();
}