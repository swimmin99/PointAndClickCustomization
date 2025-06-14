// Copyright 2025 Devhanghae All Rights Reserved.

#include "Character/CustomCharacter.h"
#include "PointAndClickCustomizing.h"               
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "SceneComponent/MeshPivotComponent.h"
#include "ActorComponent/AttachmentLoaderComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACustomCharacter::ACustomCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = false;

    GetCapsuleComponent()->SetCollisionProfileName(FName(TEXT("NoCollision")));
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    if (auto* MoveComp = GetCharacterMovement())
    {
        MoveComp->GravityScale       = 0.f;
        MoveComp->bEnablePhysicsInteraction = false;
    }
    
    // Setup camera boom (spring arm)
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->TargetArmLength = SprintArmLength;
    SpringArmComp->SetRelativeLocation(FVector(0, 0, 0));
    SpringArmComp->SetRelativeRotation(FRotator(0, 0, -90));
    SpringArmComp->bUsePawnControlRotation = false;

    // Setup camera
    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComp->SetupAttachment(SpringArmComp);
    CameraComp->SetProjectionMode(ECameraProjectionMode::Orthographic);

    // Pivot for mesh rotation
    MeshPivot = CreateDefaultSubobject<UMeshPivotComponent>(TEXT("MeshPivot"));
    MeshPivot->SetupAttachment(RootComponent);

    // Character mesh
    CustomizingMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CustomizingMesh"));
    CustomizingMesh->SetupAttachment(MeshPivot);
    CustomizingMesh->SetCollisionProfileName(FName(TEXT("NoCollision")));
    CustomizingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Editor-only debug arrow
    BottomArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("BottomArrow"));
    BottomArrow->SetupAttachment(MeshPivot);

    // Attachment loader
    AttachmentLoader = CreateDefaultSubobject<UAttachmentLoaderComponent>(TEXT("AttachmentLoader"));

    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::Constructor"), *GetName());
}

void ACustomCharacter::SetupPartsForCharacter(FName CallerID)
{
    if (!AttachmentLoader)
    {
        UE_LOG(LogCustomizingPlugin, Warning,
            TEXT("%s::SetupPartsForCharacter - No AttachmentLoader"), *GetName());
        return;
    }

    // Load attachments onto the mesh
    if (USkeletalMeshComponent* TargetMesh = GetCustomizingMesh())
    {
        AttachmentLoader->LoadExistingAttachments(TargetMesh);
        UE_LOG(LogCustomizingPlugin, Log,
            TEXT("%s::SetupPartsForCharacter - Loaded attachments for CallerID=%s"),
            *GetName(), *CallerID.ToString());
    }
}

void ACustomCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    SetReplicateMovement(false);
}

void ACustomCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (!SpringArmComp) return;
    SpringArmComp->SetComponentTickEnabled(false);  
    SpringArmComp->SetActive(false, false); 
    
    // Set mesh visibility: only owner sees their own character mesh
    if (CustomizingMesh)
    {
        CustomizingMesh->SetOnlyOwnerSee(true);
        CustomizingMesh->SetOwnerNoSee(false);
    }
    UE_LOG(LogCustomizingPlugin, Log, TEXT("%s::BeginPlay"), *GetName());
}
