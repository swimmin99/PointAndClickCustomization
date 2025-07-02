// Copyright 2025 Devhanghae All Rights Reserved.
#include "Character/BattleCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "Actor/ProjectileActor.h"
#include "Actor/ShootableActor.h"
#include "Data/BattleControlData.h"
#include "ActorComponent/AttachmentLoaderComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"


ABattleCharacter::ABattleCharacter()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -100.0f), FRotator(0.0f, -90.0f, 0.0f));
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	AttachmentLoader = CreateDefaultSubobject<UAttachmentLoaderComponent>(TEXT("AttachmentLoader"));

	bIsInputEnabled = true;
}


void ABattleCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetCharacterControl(QuaterControlData);
}

void ABattleCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (HasAuthority())
	{
		SetupPartsForCharacter(GetFName());
	}
}


void ABattleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);
	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABattleCharacter::HandleMove);
	EIC->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ABattleCharacter::HandleAttack);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->ClearAllMappings();
			if (CharacterInputMappingContext)
			{
				Subsystem->AddMappingContext(CharacterInputMappingContext, 0);
			}
		}
	}
}


void ABattleCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABattleCharacter, AttackCooldown);
	DOREPLIFETIME(ABattleCharacter, bCanAttack);
	DOREPLIFETIME(ABattleCharacter, bIsInputEnabled);
}

void ABattleCharacter::SetCharacterControl(const UBattleControlData* ControlData)
{
	if (!ControlData)
	{
		UE_LOG(LogTemp, Warning, TEXT("ABattleCharacter::SetCharacterControl: ControlData is null."));
		return;
	}

	bUseControllerRotationYaw = ControlData->bUseControllerRotationYaw;
	GetCharacterMovement()->bOrientRotationToMovement = ControlData->bOrientRotationToMovement;
	GetCharacterMovement()->bUseControllerDesiredRotation = ControlData->bUseControllerDesiredRotation;
	GetCharacterMovement()->RotationRate = ControlData->RotationRate;
	
	CameraBoom->TargetArmLength = ControlData->TargetArmLength;
	CameraBoom->SetRelativeRotation(ControlData->RelativeRotation);
	CameraBoom->bUsePawnControlRotation = ControlData->bUsePawnControlRotation;
	CameraBoom->bInheritPitch = ControlData->bInheritPitch;
	CameraBoom->bInheritYaw = ControlData->bInheritYaw;
	CameraBoom->bInheritRoll = ControlData->bInheritRoll;
	CameraBoom->bDoCollisionTest = ControlData->bDoCollisionTest;
}

void ABattleCharacter::HandleMove(const FInputActionValue& Value)
{
	if (!bIsInputEnabled || !bCanAttack) return;

	const FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();
	const FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);
	
	if (Controller)
	{
		Controller->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	}
	AddMovementInput(MoveDirection, 1.0f);
}

void ABattleCharacter::HandleAttack()
{
	if (bCanAttack && bIsInputEnabled)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult HitResult;
			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
			{
				// 서버에 공격 명령 RPC 전송
				Server_PerformAttack(HitResult.Location);
			}
		}
	}
}

void ABattleCharacter::ApplyDamage_Implementation(float DamageAmount, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return;
	if (!bIsInputEnabled) return; 

	UE_LOG(LogTemp, Log, TEXT("%s took %f damage from %s"), *GetName(), DamageAmount, *DamageCauser->GetName());

	bIsInputEnabled = false;

	Multicast_OnDeath();

	GetWorldTimerManager().SetTimer(RespawnTimer, this, &ABattleCharacter::Respawn, RespawnTime, false);
}

void ABattleCharacter::Multicast_OnDeath_Implementation()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

void ABattleCharacter::Respawn()
{
	SetActorEnableCollision(true);
	SetActorHiddenInGame(false);
	
	bIsInputEnabled = true;
}

void ABattleCharacter::Server_PerformAttack_Implementation(FVector_NetQuantize TargetLocation)
{
	if (!bCanAttack || !CachedShooter || !ProjectileClass)
	{
		return;
	}
	
	bCanAttack = false;
	OnRep_CanAttack(); 
	GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &ABattleCharacter::ResetAttackCooldown, AttackCooldown, false);

	const FVector LookDirection = (TargetLocation - GetActorLocation()).GetSafeNormal2D();
	SetActorRotation(LookDirection.Rotation());

	const FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.0f + FVector(0, 0, ProjectileSpawnZOffset);

	Multicast_PlayAttackFX(SpawnLocation, TargetLocation);

	SpawnProjectile(TargetLocation);
}

void ABattleCharacter::SpawnProjectile(const FVector& TargetLocation)
{
	if (!HasAuthority()) return;

	UStaticMesh* MeshToUse = CachedShooter->GetProjectileMesh();
	if (!MeshToUse) 
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnProjectile failed: MeshToUse is nullptr."));
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.0f + FVector(0, 0, ProjectileSpawnZOffset);
	const FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this; 
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (AProjectileActor* SpawnedProjectile = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams))
	{
		SpawnedProjectile->SetProjectileMesh(MeshToUse);
	}
}

void ABattleCharacter::Multicast_PlayAttackFX_Implementation(FVector_NetQuantize MuzzleLocation, FVector_NetQuantize TargetLocation)
{
	if (IsLocallyControlled() && !HasAuthority()) // 로컬 플레이어이며, 서버가 아닐 때
	{
		if (ProjectileClass && CachedShooter)
		{
			UStaticMesh* MeshToUse = CachedShooter->GetProjectileMesh();
			if (!MeshToUse) return;

			const FRotator SpawnRotation = (TargetLocation - MuzzleLocation).Rotation();
			
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetController();
			SpawnParams.Instigator = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			AProjectileActor* CosmeticProjectile = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass, MuzzleLocation, SpawnRotation, SpawnParams);
			if (CosmeticProjectile)
			{
				CosmeticProjectile->SetRole(ROLE_None); 
				CosmeticProjectile->SetReplicates(false);
				CosmeticProjectile->SetProjectileMesh(MeshToUse);
				if (CosmeticProjectile->GetCollisionComponent())
				{
					CosmeticProjectile->GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
		}
	}
}

void ABattleCharacter::ResetAttackCooldown()
{
	bCanAttack = true;
	OnRep_CanAttack();
}

void ABattleCharacter::OnRep_CanAttack()
{
}


void ABattleCharacter::SetupPartsForCharacter(FName CallerID)
{
	if (!HasAuthority() || !AttachmentLoader) return;

	if (USkeletalMeshComponent* TargetMesh = GetMesh())
	{
		AttachmentLoader->LoadExistingAttachments(TargetMesh);

		for (const auto& WeakAttachment : GetSpawnedAttachments())
		{
			if (WeakAttachment.IsValid())
			{
				WeakAttachment->DisableCollision();
				if (AShootableActor* Shooter = Cast<AShootableActor>(WeakAttachment.Get()))
				{
					CachedShooter = Shooter;
					UE_LOG(LogTemp, Log, TEXT("%s: Found and cached ShootableActor: %s"), *GetName(), *Shooter->GetName());
					break; 
				}
			}
		}
	}
}

const TArray<TWeakObjectPtr<AAttachableActor>>& ABattleCharacter::GetSpawnedAttachments() const
{
	if (AttachmentLoader)
	{
		return AttachmentLoader->GetAttachedActors();
	}
	
	static const TArray<TWeakObjectPtr<AAttachableActor>> EmptyArray;
	return EmptyArray;
}