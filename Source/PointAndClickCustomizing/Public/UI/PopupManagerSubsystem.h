// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/FPopupData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PopupManagerSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API UPopupManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	void RequestShowPopup(const FPopupData& InData);

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void ProcessQueue();

	UFUNCTION()
	void OnCurrentPopupConfirmed();

	UFUNCTION()
	void OnCurrentPopupCancelled();
    
	void CloseCurrentPopupAndProcessNext();

	UPROPERTY()
	TSubclassOf<class UPopupWiddgetBase> PopupWidgetClass;

	UPROPERTY()
	TObjectPtr<UPopupWiddgetBase> CurrentPopupWidget;

	TQueue<FPopupData> PopupQueue;

	FPopupDelegate CurrentConfirmDelegate;
	FPopupDelegate CurrentCancelDelegate;
};