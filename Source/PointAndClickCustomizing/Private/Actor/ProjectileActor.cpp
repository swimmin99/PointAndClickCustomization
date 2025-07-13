// Copyright 2025 Devhanghae All Rights Reserved.

#include "Actor/ProjectileActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interface/DamagableIneterface.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

AProjectileActor::AProjectileActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	
	bNetUseOwnerRelevancy = true;
	bOnlyRelevantToOwner = false;
	
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	CollisionComponent->SetSphereRadius(5.0f);

	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(RootComponent);
	ProjectileMovementComponent->InitialSpeed = 2000.0f;
	ProjectileMovementComponent->MaxSpeed = 2000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 3.0f;

	if (HasAuthority())
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectileActor::OnOverlapBegin);
		CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectileActor::OnHit);
	}
}

void AProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	if (APawn* MyInstigator = GetInstigator())
	{
		CollisionComponent->IgnoreActorWhenMoving(MyInstigator, true);
	}
}

void AProjectileActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectileActor, Damage);
	DOREPLIFETIME(AProjectileActor, ProjectileMeshToUse);
}

bool AProjectileActor::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	const AController* InstigatorController = GetInstigatorController();

	if (InstigatorController == RealViewer)
	{
		return false;
	}

	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}

void AProjectileActor::SetProjectileMesh(UStaticMesh* NewMesh)
{
	if (!HasAuthority())
	{
		if (NewMesh)
		{
			MeshComponent->SetStaticMesh(NewMesh);
		}
		return;
	}

	ProjectileMeshToUse = NewMesh;
	
	OnRep_ProjectileMesh();
}

void AProjectileActor::OnRep_ProjectileMesh()
{
	if (ProjectileMeshToUse)
	{
		MeshComponent->SetStaticMesh(ProjectileMeshToUse);
	}
}

void AProjectileActor::SetDamage(float NewDamage)
{
    if (HasAuthority())
	{
		Damage = NewDamage;
	}
}

void AProjectileActor::multiplyDamage(float Multiplier)
{
	if (HasAuthority())
	{
		Damage = Damage*=Multiplier;
	}
}

void AProjectileActor::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	Destroy();
}


void AProjectileActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor || OtherActor == this || OtherActor == GetInstigator())
	{
		return;
	}

	if (OtherActor->GetClass()->ImplementsInterface(UDamagableIneterface::StaticClass()))
	{
		IDamagableIneterface::Execute_ApplyDamage(OtherActor, Damage, GetInstigatorController(), this);

		Destroy();
	}
}