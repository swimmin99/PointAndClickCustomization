// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UITextConstants.generated.h"

UCLASS()
class POINTANDCLICKCUSTOMIZING_API UUITextConstants : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "UI|TextConstants")
	static FText GetWaitingForPlayers_Title();

	UFUNCTION(BlueprintPure, Category = "UI|TextConstants")
	static FText GetWaitingForPlayers_Description();

	UFUNCTION(BlueprintPure, Category = "UI|TextConstants")
	static FText GetConfirmReady_Title();

	UFUNCTION(BlueprintPure, Category = "UI|TextConstants")
	static FText GetConfirmReady_Description();


	UFUNCTION(BlueprintPure, Category = "UI|TextConstants")
	static FText GetHowToUse_Title();

	UFUNCTION(BlueprintPure, Category = "UI|TextConstants")
	static FText GetHowToUse_Description();
};