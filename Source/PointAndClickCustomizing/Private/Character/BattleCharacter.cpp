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
#include "ActorComponent/WeaponMapComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

ABattleCharacter::ABattleCharacter()
{
	WeaponMapComponent = CreateDefaultSubobject<UWeaponMapComponent>(TEXT("WeaponMapComponent"));

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
	EIC->BindAction(WheelAction, ETriggerEvent::Triggered, this, &ABattleCharacter::HandleChangeWeapon);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
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
	DOREPLIFETIME(ABattleCharacter, bIsDead);
}

void ABattleCharacter::Server_RequestDeathValidation_Implementation(uint32 PredictionId, float ReportedDamage, AActor* DamageCauser)
{
	if (GetOwner() != Cast<AActor>(GetController())) return;

	const bool bActuallyDead = /* Health <= 0 ? or ApplyDamageThenCheck ? */ false;

	if (bActuallyDead)
	{
		if (!bIsDead)
		{
			bIsDead = true;
			FlushNetDormancy(); ForceNetUpdate();
		}
		Client_DeathValidationResult(PredictionId, /*bConfirmed*/true);
	}
	else
	{
		Client_DeathValidationResult(PredictionId, /*bConfirmed*/false);
	}
}

void ABattleCharacter::Client_DeathValidationResult_Implementation(uint32 PredictionId, bool bConfirmed)
{
	if (PredictionId != LastPredictionId) return; // 구버전 응답 무시

	GetWorldTimerManager().ClearTimer(MockDeathTimeout);

	if (bConfirmed)
	{
		if (!bIsDead) { bIsDead = true; OnRep_IsDead(); }
	}
	else
	{
		if (bIsDead) { bIsDead = false; OnRep_IsDead(); }
	}
}

void ABattleCharacter::OnRep_IsDead()
{
	const bool bCharacterDead = bIsDead;

	SetActorHiddenInGame(bCharacterDead);
	SetActorEnableCollision(!bCharacterDead);

	if (USkeletalMeshComponent* Skel = GetMesh())
	{
		Skel->SetHiddenInGame(bCharacterDead, true);
		Skel->SetVisibility(!bCharacterDead, true);
	}

	if (UCapsuleComponent* Cap = GetCapsuleComponent())
		Cap->SetCollisionEnabled(bCharacterDead ? ECollisionEnabled::NoCollision
										 : ECollisionEnabled::QueryAndPhysics);

	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (bIsDead) DisableInput(PC);
			else         EnableInput(PC);
		}
	}
}


void ABattleCharacter::ApplyDamageForMock_Implementation(float DamageAmount, AController*, AActor* Causer)
{
	if (bIsDead) return;

	bIsDead = true;
	OnRep_IsDead();

	const uint32 Pid = ++LastPredictionId;

	Server_RequestDeathValidation(Pid, DamageAmount, Causer);

	GetWorldTimerManager().SetTimer(
		MockDeathTimeout, [this, Pid]()
		{
			if (LastPredictionId == Pid && bIsDead)
			{
				UE_LOG(LogTemp, Warning, TEXT("[MockTimeout] rollback prediction %u"), Pid);
				bIsDead = false;
				OnRep_IsDead();
			}
		},
		0.35f, false); 
}

void ABattleCharacter::ApplyDamage_Implementation(float DamageAmount, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || bIsDead) return;

	UE_LOG(LogTemp, Log, TEXT("%s took %f damage from %s"),
		*GetName(), DamageAmount, *GetNameSafe(DamageCauser));

	bIsDead = true;

	FlushNetDormancy();
	ForceNetUpdate();

	GetWorldTimerManager().SetTimer(RespawnTimer, this, &ABattleCharacter::Respawn, RespawnTime, false);
}

void ABattleCharacter::Respawn()
{
	bIsDead = false;

	FlushNetDormancy();
	ForceNetUpdate();
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

	GetCharacterMovement()->SetIsReplicated(false);
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
					WeaponMapComponent->AddWeapon(Shooter);
				}
			}
		}
	}
}

void ABattleCharacter::HandleMove(const FInputActionValue& Value)
{
	if (bIsDead || !bCanAttack) return;

	const FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();
	const FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.0f);

	if (Controller)
	{
		Controller->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	}
	AddMovementInput(MoveDirection, 1.0f);
}

void ABattleCharacter::HandleChangeWeapon(const FInputActionValue& Value)
{
	if (!WeaponMapComponent) return;

	const float WheelDirection = Value.Get<float>();
	if (WheelDirection > 0.f)
	{
		WeaponMapComponent->Server_FocusNextWeapon();
		UE_LOG(LogTemp, Log, TEXT("Requested Server_FocusNextWeapon"));
	}
	else if (WheelDirection < 0.f)
	{
		WeaponMapComponent->Server_FocusPreviousWeapon();
		UE_LOG(LogTemp, Log, TEXT("Requested Server_FocusPreviousWeapon"));
	}
}

void ABattleCharacter::HandleAttack()
{
	UE_LOG(LogTemp, Log, TEXT("ABattleCharacter::HandleAttack: Fire Weapon Pressed."));

	if (bCanAttack && !bIsDead)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult HitResult;
			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
			{
				UE_LOG(LogTemp, Log, TEXT("ABattleCharacter::HandleAttack: Fire Weapon Called."));
				Server_PerformAttack(HitResult.Location);
				Solo_PlayAttackFX(HitResult.Location); // 로컬 FX
			}
		}
	}
}

void ABattleCharacter::Server_PerformAttack_Implementation(FVector_NetQuantize TargetLocation)
{
	if (!bCanAttack || !WeaponMapComponent || !WeaponMapComponent->IsValid() || !ProjectileClass)
	{
		if (!bCanAttack) UE_LOG(LogTemp, Warning, TEXT(" - Reason: bCanAttack is false."));
		if (!WeaponMapComponent) UE_LOG(LogTemp, Warning, TEXT(" - Reason: MyWeaponMap is nullptr."));
		if (!WeaponMapComponent->IsValid()) UE_LOG(LogTemp, Warning, TEXT(" - Reason: MyWeaponMap->IsValid() returned false."));
		if (!ProjectileClass) UE_LOG(LogTemp, Warning, TEXT(" - Reason: ProjectileClass is nullptr."));
		return;
	}

	bCanAttack = false;
	OnRep_CanAttack();
	GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &ABattleCharacter::ResetAttackCooldown, AttackCooldown, false);

	const FVector LookDirection = (TargetLocation - GetActorLocation()).GetSafeNormal2D();
	SetActorRotation(LookDirection.Rotation());

	SpawnProjectile(TargetLocation);
}

void ABattleCharacter::SpawnProjectile(const FVector& TargetLocation)
{
	if (!HasAuthority()) return;

	float_t multiplier = 1.f;
	if (ProjectileClass && WeaponMapComponent->IsValid())
	{
		for (const auto& Shooter : WeaponMapComponent->GetCurrentWeapons())
		{
			ShootProjectile(TargetLocation, Shooter, true, [=](AProjectileActor* SpawnedProjectile)
				{
					SpawnedProjectile->multiplyDamage(multiplier);
				});
		}
	}
}

void ABattleCharacter::Solo_PlayAttackFX(FVector_NetQuantize TargetLocation)
{
	if (IsLocallyControlled() && !HasAuthority())
	{
		if (ProjectileClass && WeaponMapComponent->IsValid())
		{
			for (const auto& Shooter : WeaponMapComponent->GetCurrentWeapons())
			{
				if (Shooter)
				{
					Shooter->SetRole(ROLE_None);
					Shooter->SetReplicates(false);

					ShootProjectile(TargetLocation, Shooter, false, [](AProjectileActor* SpawnedProjectile)
						{
						});
				}
			}
		}
	}
}

void ABattleCharacter::ShootProjectile(FVector_NetQuantize TargetLocation, TObjectPtr<AShootableActor> Projectile, bool isAuthority, TFunction<void(AProjectileActor*)> Callback)
{
	UStaticMesh* MeshToUse = Projectile ? Projectile->GetProjectileMesh() : nullptr;
	if (!MeshToUse) return;

	const FRotator SpawnRotation = (TargetLocation - Projectile->GetActorLocation()).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = isAuthority
		? ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		: ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FTransform SpawnTransform(SpawnRotation, Projectile->GetActorLocation(), FVector(0.1f, 0.1f, 0.1f));
	AProjectileActor* ProjectileActor = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass, SpawnTransform, SpawnParams);
	if (!ProjectileActor) return;

	ProjectileActor->SetProjectileMesh(MeshToUse);

	if (Callback)
	{
		Callback(ProjectileActor);
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

const TArray<TWeakObjectPtr<AAttachableActor>>& ABattleCharacter::GetSpawnedAttachments() const
{
	if (AttachmentLoader)
	{
		return AttachmentLoader->GetAttachedActors();
	}

	static const TArray<TWeakObjectPtr<AAttachableActor>> EmptyArray;
	return EmptyArray;
}