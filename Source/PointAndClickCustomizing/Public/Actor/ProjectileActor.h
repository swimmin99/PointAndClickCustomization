// Copyright 2025 Devhanghae All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileActor.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UStaticMesh;

/**
 * A replicated projectile actor.
 * Handles movement, collision, and damage application on the server.
 * Supports a client-side prediction model where the owning client has a local cosmetic version.
 */
UCLASS()
class POINTANDCLICKCUSTOMIZING_API AProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileActor();

	USphereComponent* GetCollisionComponent() const { return CollisionComponent; }
	void SetProjectileMesh(UStaticMesh* NewMesh);
	void SetDamage(float NewDamage);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(ReplicatedUsing = OnRep_ProjectileMesh)
	TObjectPtr<UStaticMesh> ProjectileMeshToUse;
	
	UFUNCTION()
	void OnRep_ProjectileMesh();

private:
	UPROPERTY(Replicated)
	float Damage;
};