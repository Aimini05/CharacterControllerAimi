#include "CustomPlayerPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"

ACustomPlayerPawn::ACustomPlayerPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
    Capsule->SetupAttachment(Root);
    Capsule->SetCapsuleHalfHeight(88.f);
    Capsule->SetCapsuleRadius(34.f);
    Capsule->SetCollisionProfileName(TEXT("Pawn"));

    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
    VisualMesh->SetupAttachment(Capsule);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualMesh->SetRelativeLocation(FVector(0.f, 0.f, -88.f));

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(Capsule);
    SpringArm->TargetArmLength = 300.f;
    SpringArm->bDoCollisionTest = false; // vi gör egen kamerahantering senare om vi vill
    SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 60.f));

    ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
    ThirdPersonCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(Capsule);
    FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));

    ThirdPersonCamera->SetActive(true);
    FirstPersonCamera->SetActive(false);
}

void ACustomPlayerPawn::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                if (DefaultMappingContext)
                {
                    Subsystem->AddMappingContext(DefaultMappingContext, 0);
                }
            }
        }
    }
}

void ACustomPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput) return;

    if (MoveAction)
    {
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACustomPlayerPawn::Input_Move);
        EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACustomPlayerPawn::Input_Move);
    }

    if (LookAction)
    {
        EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACustomPlayerPawn::Input_Look);
    }

    if (JumpAction)
    {
        EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACustomPlayerPawn::Input_JumpStarted);
    }

    if (ToggleCameraAction)
    {
        EnhancedInput->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &ACustomPlayerPawn::Input_ToggleCamera);
    }
}

void ACustomPlayerPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateState(DeltaTime);
    UpdateCamera(DeltaTime);
    UpdateMovement(DeltaTime);
    SolveCollisionAndMove(DeltaTime);
    Depenetrate();

    bJumpPressed = false;
}

void ACustomPlayerPawn::Input_Move(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();

    // Fram/bak + höger/vänster relativt yaw
    const FRotator YawRot(0.f, GetActorRotation().Yaw, 0.f);
    const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
    const FVector Right   = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    FVector Desired = (Forward * Axis.Y) + (Right * Axis.X);

    // normalisera bara om > 1, så analog input fortfarande kan vara "mindre än full fart"
    if (Desired.SizeSquared() > 1.f)
    {
        Desired.Normalize();
    }

    MoveInput = Desired;
}

void ACustomPlayerPawn::Input_Look(const FInputActionValue& Value)
{
    const FVector2D Look = Value.Get<FVector2D>();

    YawInput += Look.X * MouseSensitivityYaw;
    PitchInput = FMath::Clamp(PitchInput + (Look.Y * MouseSensitivityPitch), MinPitch, MaxPitch);
}

void ACustomPlayerPawn::Input_JumpStarted(const FInputActionValue& Value)
{
    bJumpPressed = true;
}

void ACustomPlayerPawn::Input_ToggleCamera(const FInputActionValue& Value)
{
    bUseThirdPerson = !bUseThirdPerson;
    ThirdPersonCamera->SetActive(bUseThirdPerson);
    FirstPersonCamera->SetActive(!bUseThirdPerson);
}

void ACustomPlayerPawn::UpdateState(float DeltaTime)
{
    const bool bGrounded = IsGrounded();

    if (bGrounded)
    {
        if (bJumpPressed)
        {
            CurrentState = EPlayerMovementState::Jumping;
        }
        else
        {
            CurrentState = EPlayerMovementState::Grounded;
        }
    }
    else
    {
        CurrentState = Velocity.Z > 0.f
            ? EPlayerMovementState::Jumping
            : EPlayerMovementState::Falling;
    }
}

void ACustomPlayerPawn::UpdateCamera(float DeltaTime)
{
    // Pawnens yaw
    FRotator ActorRot = GetActorRotation();
    ActorRot.Yaw = YawInput;
    SetActorRotation(ActorRot);

    // Kamerans pitch
    if (bUseThirdPerson)
    {
        SpringArm->SetRelativeRotation(FRotator(PitchInput, 0.f, 0.f));
    }
    else
    {
        FirstPersonCamera->SetRelativeRotation(FRotator(PitchInput, 0.f, 0.f));
    }
}

void ACustomPlayerPawn::UpdateMovement(float DeltaTime)
{
    // Gravity
    Velocity.Z -= Gravity * DeltaTime;
    Velocity.Z = FMath::Clamp(Velocity.Z, -MaxFallSpeed, MaxFallSpeed);

    // Jump
    if (CurrentState == EPlayerMovementState::Jumping && IsGrounded())
    {
        Velocity.Z = JumpImpulse;
    }

    // Horisontell acceleration
    ApplyHorizontalAcceleration(DeltaTime);

    // Friktion på mark
    if (CurrentState == EPlayerMovementState::Grounded && MoveInput.IsNearlyZero())
    {
        ApplyGroundFriction(DeltaTime);
    }

    ClampHorizontalSpeed();
}

void ACustomPlayerPawn::ApplyHorizontalAcceleration(float DeltaTime)
{
    if (MoveInput.IsNearlyZero())
        return;

    const float Control = (CurrentState == EPlayerMovementState::Grounded) ? 1.f : AirControl;
    FVector Horizontal = GetHorizontalVelocity();
    Horizontal += MoveInput * Acceleration * Control * DeltaTime;
    SetHorizontalVelocity(Horizontal);
}

void ACustomPlayerPawn::ApplyGroundFriction(float DeltaTime)
{
    FVector Horizontal = GetHorizontalVelocity();
    const float Speed = Horizontal.Size();
    if (Speed <= KINDA_SMALL_NUMBER)
    {
        SetHorizontalVelocity(FVector::ZeroVector);
        return;
    }

    const float Drop = GroundFriction * Acceleration * DeltaTime;
    const float NewSpeed = FMath::Max(0.f, Speed - Drop);
    Horizontal = Horizontal.GetSafeNormal() * NewSpeed;
    SetHorizontalVelocity(Horizontal);
}

void ACustomPlayerPawn::ClampHorizontalSpeed()
{
    FVector Horizontal = GetHorizontalVelocity();
    const float Speed = Horizontal.Size();
    if (Speed > MaxGroundSpeed)
    {
        Horizontal = Horizontal.GetSafeNormal() * MaxGroundSpeed;
        SetHorizontalVelocity(Horizontal);
    }
}

void ACustomPlayerPawn::SolveCollisionAndMove(float DeltaTime)
{
    int32 Iterations = 0;
    const int32 MaxIterations = 6;

    while (Iterations < MaxIterations)
    {
        const FVector Movement = Velocity * DeltaTime;
        if (Movement.IsNearlyZero())
        {
            return;
        }

        FVector Origin = Capsule->GetComponentLocation();
        const FQuat Rotation = Capsule->GetComponentQuat();

        const FVector Direction = Movement.GetSafeNormal();
        const float Distance = Movement.Size();

        const FVector TraceStart = Origin;
        const FVector TraceEnd = TraceStart + Direction * (Distance + ColliderMargin);

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        FHitResult Hit;
        const bool bHit = GetWorld()->SweepSingleByChannel(
            Hit,
            TraceStart,
            TraceEnd,
            Rotation,
            TraceChannel,
            FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()),
            Params
        );

        if (!bHit)
        {
            SetActorLocation(GetActorLocation() + Movement);
            return;
        }

        const float Dot = FVector::DotProduct(Direction, Hit.Normal);
        if (FMath::IsNearlyZero(Dot))
        {
            return;
        }

        const float DistanceToColliderNeg = ColliderMargin / Dot;
        const float AllowedDistance = Hit.Distance + DistanceToColliderNeg;

        if (AllowedDistance > 0.f)
        {
            SetActorLocation(GetActorLocation() + Direction * AllowedDistance);
        }

        const FVector NormalForce = CalculateNormalForce(Velocity, Hit.Normal);
        Velocity += NormalForce;

        // Om vi landar: slå ut nedåtkraft
        if (Hit.Normal.Z > 0.5f && Velocity.Z < 0.f)
        {
            Velocity.Z = 0.f;
        }

        Iterations++;
    }
}

bool ACustomPlayerPawn::IsGrounded(FHitResult* OutHit) const
{
    const FVector Start = Capsule->GetComponentLocation();
    const FVector End = Start + FVector(0.f, 0.f, -(GroundCheckDistance + ColliderMargin));

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    FHitResult Hit;
    const bool bHit = GetWorld()->SweepSingleByChannel(
        Hit,
        Start,
        End,
        Capsule->GetComponentQuat(),
        TraceChannel,
        FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()),
        Params
    );

    if (OutHit)
    {
        *OutHit = Hit;
    }

    return bHit;
}

FVector ACustomPlayerPawn::GetHorizontalVelocity() const
{
    return FVector(Velocity.X, Velocity.Y, 0.f);
}

void ACustomPlayerPawn::SetHorizontalVelocity(const FVector& NewHorizontal)
{
    Velocity.X = NewHorizontal.X;
    Velocity.Y = NewHorizontal.Y;
}

FVector ACustomPlayerPawn::CalculateNormalForce(const FVector& Force, const FVector& Normal) const
{
    const float Dot = FVector::DotProduct(Force, Normal);
    const FVector Projection = Dot < 0.f ? Dot * Normal : FVector::ZeroVector;
    return -Projection;
}

void ACustomPlayerPawn::Depenetrate()
{
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    const FVector Origin = Capsule->GetComponentLocation();

    const bool bOverlapping = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Origin,
        Capsule->GetComponentQuat(),
        TraceChannel,
        FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight()),
        Params
    );

    if (!bOverlapping) return;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        if (!Overlap.Component.IsValid()) continue;

        FMTDResult MTD;
        const bool bHasPenetration = Overlap.Component->ComputePenetration(
            MTD,
            FCollisionShape::MakeCapsule(
                Capsule->GetScaledCapsuleRadius() + ColliderMargin,
                Capsule->GetScaledCapsuleHalfHeight() + ColliderMargin),
            Origin,
            Capsule->GetComponentQuat()
        );

        if (bHasPenetration)
        {
            SetActorLocation(GetActorLocation() + MTD.Direction * (MTD.Distance + ColliderMargin));

            const FVector NormalForce = CalculateNormalForce(Velocity, -MTD.Direction);
            Velocity += NormalForce;
            break;
        }
    }
}