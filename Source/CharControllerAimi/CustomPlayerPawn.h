#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "CustomPlayerPawn.generated.h"

class UCapsuleComponent;
class UCameraComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UInputMappingContext;
class UInputAction;

UENUM(BlueprintType)
enum class EPlayerMovementState : uint8
{
    Grounded,
    Jumping,
    Falling
};

UCLASS()
class YOURPROJECT_API ACustomPlayerPawn : public APawn
{
    GENERATED_BODY()

public:
    ACustomPlayerPawn();

    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
    virtual void BeginPlay() override;

private:
    // =========================
    // Components
    // =========================
    UPROPERTY(VisibleAnywhere, Category="Components")
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere, Category="Components")
    UCapsuleComponent* Capsule;

    UPROPERTY(VisibleAnywhere, Category="Components")
    UStaticMeshComponent* VisualMesh;

    UPROPERTY(VisibleAnywhere, Category="Components")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, Category="Components")
    UCameraComponent* ThirdPersonCamera;

    UPROPERTY(VisibleAnywhere, Category="Components")
    UCameraComponent* FirstPersonCamera;

    // =========================
    // Enhanced Input
    // =========================
    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* MoveAction;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* LookAction;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* JumpAction;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    UInputAction* ToggleCameraAction;

    // =========================
    // Movement tuning
    // =========================
    UPROPERTY(EditAnywhere, Category="Movement")
    float Acceleration = 2400.f;

    UPROPERTY(EditAnywhere, Category="Movement")
    float MaxGroundSpeed = 900.f;

    UPROPERTY(EditAnywhere, Category="Movement")
    float GroundFriction = 8.f;

    UPROPERTY(EditAnywhere, Category="Movement")
    float AirControl = 0.45f;

    UPROPERTY(EditAnywhere, Category="Movement")
    float Gravity = 2400.f;

    UPROPERTY(EditAnywhere, Category="Movement")
    float MaxFallSpeed = 3200.f;

    UPROPERTY(EditAnywhere, Category="Movement")
    float JumpImpulse = 950.f;

    UPROPERTY(EditAnywhere, Category="Movement")
    float GroundCheckDistance = 8.f;

    UPROPERTY(EditAnywhere, Category="Movement")
    float ColliderMargin = 2.f;

    UPROPERTY(EditAnywhere, Category="Movement")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

    // =========================
    // Camera tuning
    // =========================
    UPROPERTY(EditAnywhere, Category="Camera")
    float MouseSensitivityYaw = 1.0f;

    UPROPERTY(EditAnywhere, Category="Camera")
    float MouseSensitivityPitch = 1.0f;

    UPROPERTY(EditAnywhere, Category="Camera")
    float MinPitch = -80.f;

    UPROPERTY(EditAnywhere, Category="Camera")
    float MaxPitch = 80.f;

    // =========================
    // Runtime data
    // =========================
    FVector MoveInput = FVector::ZeroVector;
    FVector Velocity = FVector::ZeroVector;

    float YawInput = 0.f;
    float PitchInput = 0.f;

    bool bUseThirdPerson = true;
    bool bJumpPressed = false;

    EPlayerMovementState CurrentState = EPlayerMovementState::Falling;

private:
    // Input handlers
    void Input_Move(const FInputActionValue& Value);
    void Input_Look(const FInputActionValue& Value);
    void Input_JumpStarted(const FInputActionValue& Value);
    void Input_ToggleCamera(const FInputActionValue& Value);

    // Update
    void UpdateState(float DeltaTime);
    void UpdateCamera(float DeltaTime);
    void UpdateMovement(float DeltaTime);
    void SolveCollisionAndMove(float DeltaTime);
    void ApplyGroundFriction(float DeltaTime);
    void ApplyHorizontalAcceleration(float DeltaTime);
    void ClampHorizontalSpeed();

    // Helpers
    bool IsGrounded(FHitResult* OutHit = nullptr) const;
    FVector GetHorizontalVelocity() const;
    void SetHorizontalVelocity(const FVector& NewHorizontal);
    FVector CalculateNormalForce(const FVector& Force, const FVector& Normal) const;
    void Depenetrate();
};