#include "PlayerController/BattlePlayerController.h"
#include "PointAndClickCustomizing.h"
#include "EnhancedInputComponent.h"

void ABattlePlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// No per-frame logic yet
}

void ABattlePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		UE_LOG(LogCustomizingPlugin, Log, TEXT("ABattlePlayerController::SetupInputComponent - Binding Back action"));
		EIC->BindAction(IA_Back, ETriggerEvent::Triggered, this, &ThisClass::Input_Back);
	}
	else
	{
		UE_LOG(LogCustomizingPlugin, Warning, TEXT("ABattlePlayerController::SetupInputComponent - Missing EnhancedInputComponent"));
	}
}

void ABattlePlayerController::Input_Back(const FInputActionValue& Value)
{
	UE_LOG(LogCustomizingPlugin, Log, TEXT("ABattlePlayerController::Input_Back - Traveling back to %s"), *CustomizingMapPath);
	TravelToLevel(CustomizingMapPath);
}
