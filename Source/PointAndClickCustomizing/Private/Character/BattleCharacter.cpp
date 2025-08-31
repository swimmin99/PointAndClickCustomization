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
	EIC->BindAction(WheelAction, ETriggerEvent::Triggered, this, &ABattleCharacter::HandleChangeWeapon);
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
	DOREPLIFETIME(ABattleCharacter, bIsDead);
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
	if (!bIsInputEnabled || !bCanAttack) return;

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

	if (bCanAttack && bIsInputEnabled)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult HitResult;
			if (PC->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
			{
				UE_LOG(LogTemp, Log, TEXT("ABattleCharacter::HandleAttack: Fire Weapon Called."));
				Server_PerformAttack(HitResult.Location);
				Solo_PlayAttackFX(HitResult.Location);
			}
		}
	}
}

void ABattleCharacter::ApplyDamageForMock_Implementation(float DamageAmount, AController* EventInstigator, AActor* DamageCauser)
{
	if (HasAuthority()) return;

	// 이미 대기 중/확정 사망이면 중복 방지
	if (!bIsInputEnabled || bIsDead) return;

	SetActorHiddenInGame(true);
	bIsInputEnabled = false;

	GetWorldTimerManager().SetTimer(
		DeathQueryTimer,
		this,
		&ABattleCharacter::RequestDeathValidation_Deferred,
		1.0f,
		false
	);
}

void ABattleCharacter::RequestDeathValidation_Deferred()
{
	// 이미 확정 사망/부활 등 상태 변했으면 무시
	if (bIsDead)
		return;

	Server_AskForDeathValidation();
}

void ABattleCharacter::ApplyDamage_Implementation(float DamageAmount, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return;
	if (!bIsInputEnabled) return; 

	UE_LOG(LogTemp, Log, TEXT("%s took %f damage from %s"), *GetName(), DamageAmount, *DamageCauser->GetName());

	bIsInputEnabled = false;

	bIsDead = true;
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
	bIsDead = false;
	bIsInputEnabled = true;
}

void ABattleCharacter::Server_PerformAttack_Implementation(FVector_NetQuantize TargetLocation)
{
    if (!bCanAttack || !WeaponMapComponent || !WeaponMapComponent->IsValid() || !ProjectileClass)
	{
		if (!bCanAttack)
		{
			UE_LOG(LogTemp, Warning, TEXT(" - Reason: bCanAttack is false."));
		}
		if (!WeaponMapComponent)
		{
			UE_LOG(LogTemp, Warning, TEXT(" - Reason: MyWeaponMap object itself is a nullptr."));
		}
		else if (!WeaponMapComponent->IsValid()) 
		{
			UE_LOG(LogTemp, Warning, TEXT(" - Reason: MyWeaponMap->IsValid() returned false."));
		}
		if (!ProjectileClass)
		{
			UE_LOG(LogTemp, Warning, TEXT(" - Reason: ProjectileClass is a nullptr."));
		}
		
		return;
	}
	
	bCanAttack = false;
	OnRep_CanAttack(); 
	GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &ABattleCharacter::ResetAttackCooldown, AttackCooldown, false);

	const FVector LookDirection = (TargetLocation - GetActorLocation()).GetSafeNormal2D();
	SetActorRotation(LookDirection.Rotation());
	

	SpawnProjectile(TargetLocation);
}


void ABattleCharacter::Server_AskForDeathValidation_Implementation()
{
	if (!bIsDead)
		Client_DeathRejected();
}


void ABattleCharacter::SpawnProjectile(const FVector& TargetLocation)
{
	if (!HasAuthority()) return;
	float_t multiplier = 1;
	if (ProjectileClass && WeaponMapComponent->IsValid())
	{
		for(const auto & Shooter : WeaponMapComponent->GetCurrentWeapons())
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
			for(const auto & Shooter : WeaponMapComponent->GetCurrentWeapons())
			{
				if (Shooter)
				{
					Shooter->SetRole(ROLE_None); 
					Shooter->SetReplicates(false);

					ShootProjectile(TargetLocation, Shooter, false, [](AProjectileActor* SpawnedProjectile)
					{
						
						if (SpawnedProjectile->GetCollisionComponent())
						{
							SpawnedProjectile->GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						}
						
					});
				}
			}
		}
	}
}

void ABattleCharacter::ShootProjectile(FVector_NetQuantize TargetLocation, TObjectPtr <AShootableActor> Projectile, bool isAuthority, TFunction<void(AProjectileActor*)> Callback)
{
	UStaticMesh* MeshToUse = Projectile->GetProjectileMesh();
	if (!MeshToUse) return;

	const FRotator SpawnRotation = (TargetLocation - Projectile->GetActorLocation()).Rotation();
			
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	if (isAuthority)
	{
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	} else
	{
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	}
	FTransform SpawnTransform(SpawnRotation, Projectile->GetActorLocation(), FVector(0.1f, 0.1f, 0.1f));
	AProjectileActor* ProjectileActor = GetWorld()->SpawnActor<AProjectileActor>(ProjectileClass, SpawnTransform, SpawnParams);
	ProjectileActor->SetProjectileMesh(MeshToUse);

	if (Callback)
		Callback(ProjectileActor);
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