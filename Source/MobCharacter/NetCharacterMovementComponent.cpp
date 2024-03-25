#include "NetCharacterMovementComponent.h"

void UNetCharacterMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (GrappedDirection.IsZero())
    {
        return;
    }
    
    FVector NewLocation = UpdatedComponent->GetComponentLocation() + GrappedDirection * GrappedSpeed * DeltaTime;
    UpdatedComponent->SetWorldLocation(NewLocation, false);
}

UNetCharacterMovementComponent::UNetCharacterMovementComponent()
{
    InitialGravityScale = GravityScale;
}

void UNetCharacterMovementComponent::SetGrapped(FVector Direction)
{
    GrappedDirection = Direction;
    GrappedDirection.Normalize();

    if (Direction.IsZero())
    {
        SetMovementMode(MOVE_Falling);
        GravityScale = InitialGravityScale;

    }
    else
    {
        SetMovementMode(MOVE_None);
        GravityScale = 0.0f;
    }
}