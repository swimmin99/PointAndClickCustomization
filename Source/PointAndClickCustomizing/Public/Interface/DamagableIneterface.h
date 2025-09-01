// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DamagableIneterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UDamagableIneterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class POINTANDCLICKCUSTOMIZING_API IDamagableIneterface
{
	GENERATED_BODY()

	public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void ApplyDamage(float DamageAmount, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Damage")
	void ApplyDamageForMock(float DamageAmount, AController* EventInstigator, AActor* DamageCauser);
};
