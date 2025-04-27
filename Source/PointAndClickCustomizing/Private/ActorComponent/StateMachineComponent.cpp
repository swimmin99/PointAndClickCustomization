#include "ActorComponent/StateMachineComponent.h"
#include "PointAndClickCustomizing.h" 
#include "Types/ECustomizingState.h"

void UStateMachineComponent::SetState(ECustomizingState NewState)
{
	CurrentState = NewState;
}

ECustomizingState UStateMachineComponent::GetState() const
{
	return CurrentState;
}
