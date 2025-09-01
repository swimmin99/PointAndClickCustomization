// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/CutomizableCharacter.h"
#include "InputAction.h"
#include "Actor/ShootableActor.h"
#include "Interface/DamagableIneterface.h"
#include "BattleCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UBattleControlData;
class UAttachmentLoaderComponent;
class AProjectileActor;
class AShootableActor;
class AAttachableActor;
class UWeaponMapComponent;

UCLASS()
class POINTANDCLICKCUSTOMIZING_API ABattleCharacter
	: public ACharacter, public ICutomizableCharacter, public IDamagableIneterface
{
	GENERATED_BODY()

public:
	ABattleCharacter();
	virtual void SetupPartsForCharacter(FName CallerID = NAME_None) override;
	UWeaponMapComponent* GetWeaponMapComponent() const { return WeaponMapComponent; }

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	const TArray<TWeakObjectPtr<AAttachableActor>>& GetSpawnedAttachments() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputAction> WheelAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input)
	TObjectPtr<UInputMappingContext> CharacterInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = CharacterControl)
	TObjectPtr<UBattleControlData> QuaterControlData;

private:
	double PredictedDeadTime = 0.0;
	double PredictedGrace = 0.15; 

	UFUNCTION(Server, Reliable)
	void Server_RequestDeathValidation(uint32 PredictionId, float ReportedDamage, AActor* DamageCauser);

	UFUNCTION(Client, Reliable)
	void Client_DeathValidationResult(uint32 PredictionId, bool bConfirmed);
	
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	UFUNCTION()
	void OnRep_IsDead();

	void HandleMove(const FInputActionValue& Value);
	void HandleChangeWeapon(const FInputActionValue& Value);
	void HandleAttack();

	virtual void ApplyDamageForMock_Implementation(float DamageAmount, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void ApplyDamage_Implementation(float DamageAmount, AController* EventInstigator, AActor* DamageCauser) override;

	FTimerHandle RespawnTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float RespawnTime = 10.0f;

	void SetCharacterControl(const UBattleControlData* ControlData);

	UFUNCTION(Server, Reliable)
	void Server_PerformAttack(FVector_NetQuantize TargetLocation);

	void SpawnProjectile(const FVector& TargetLocation);
	void Solo_PlayAttackFX(FVector_NetQuantize TargetLocation);

	UPROPERTY(VisibleAnywhere, Category = "CustomizingPlugin|Attachment")
	TObjectPtr<UAttachmentLoaderComponent> AttachmentLoader;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWeaponMapComponent> WeaponMapComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TSubclassOf<AProjectileActor> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float ProjectileSpawnZOffset = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Attack", Replicated)
	float AttackCooldown = 1.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	bool bCanAttack = true;

	void Respawn();

	UFUNCTION()
	void OnRep_CanAttack();

	FTimerHandle AttackCooldownTimer;

	void ShootProjectile(FVector_NetQuantize TargetLocation, TObjectPtr<AShootableActor> Projectile, bool isAuthority, TFunction<void(AProjectileActor*)> Callback);
	void ResetAttackCooldown();

	uint32 LastPredictionId = 0;
	FTimerHandle MockDeathTimeout;
};