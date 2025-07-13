// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Actor/ShootableActor.h"
#include "WeaponMapComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AShootableActor>> Weapons;
};

USTRUCT()
struct FWeaponMapEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FName WeaponID;

	UPROPERTY()
	FWeaponArray WeaponArray;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class POINTANDCLICKCUSTOMIZING_API UWeaponMapComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponMapComponent();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	TMap<FName, FWeaponArray> WeaponMap;
    
	UPROPERTY(ReplicatedUsing = OnRep_WeaponList)
	TArray<FWeaponMapEntry> ReplicatedWeaponList;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponID)
	FName CurrentWeaponID = NAME_None;

	UFUNCTION()
	void OnRep_WeaponList();

	UFUNCTION()
	void OnRep_CurrentWeaponID();

private:
	void UpdateReplicatedList();

public:
	void AddWeapon(AShootableActor* TargetActor);

	UFUNCTION(Server, Reliable)
	void Server_FocusNextWeapon();
	UFUNCTION(Server, Reliable)
	void Server_FocusPreviousWeapon();

	AShootableActor* GetCurrentWeapon() const;
	TArray<TObjectPtr<AShootableActor>> GetCurrentWeapons() const;
	bool IsValid() const;
};