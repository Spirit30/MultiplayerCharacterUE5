#include "NetCharacter.h"
#include "GameFramework/PlayerState.h"
#include "Components/CapsuleComponent.h"
#include <Net/UnrealNetwork.h>

ANetCharacter::ANetCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNetCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANetCharacter::BeginPlay()
{
	Super::BeginPlay();

    Camera = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;

    DirectionMarkerContainer = GetCapsuleComponent()->GetOwner()->FindComponentByClass<UStaticMeshComponent>();
    DirectionMarkerView = Cast<UStaticMeshComponent>(DirectionMarkerContainer->GetChildComponent(0));

    IsServer = GetNetMode() == NM_ListenServer;
    IsClient = GetNetMode() == NM_Client;

    if (!IsServer && !IsClient)
    {
        UE_LOG(LogClass, Log, TEXT("ANetCharacter::NetMode in the Editor should be: ListenServer"));
        return;
    }
}

void ANetCharacter::Move(float x, float y)
{
	if (IsGrapped)
	{
        UE_LOG(LogTemp, Log, TEXT("ANetCharacter::Cant Move IsGrapped: %d"), IsGrapped);
        return;
    }

	const auto Direction = GetActorRotation().RotateVector(FVector(x, y, 0));
	AddMovementInput(Direction);
}

void ANetCharacter::Look(float x, float y)
{
	if (IsGrapped)
	{
		return;
	}

    AddControllerYawInput(x);
    AddControllerPitchInput(y);
}

void ANetCharacter::Grapped()
{
    if (IsGrapped)
    {
        return;
    }

    const auto Destination = SampleGrappedDestination();
    GrappedFly(Destination);

    if (IsClient)
    {
        GrappedFlyOnServer(Destination);
    }
    else if (IsServer)
    {
        GrappedFlyOnClient(Destination);
    }
}

void ANetCharacter::GrappedFlyOnServer_Implementation(FVector Destination)
{
    //UE_LOG(LogTemp, Log, TEXT("ANetCharacter::GrappedFlyServer_Implementation IsServer: %d"), IsServer);
    GrappedFly(Destination);
}

void ANetCharacter::GrappedFlyOnClient_Implementation(FVector Destination)
{
    //UE_LOG(LogTemp, Log, TEXT("ANetCharacter::GrappedFlyClient_Implementation IsClient: %d"), IsClient);
    GrappedFly(Destination);
}

void ANetCharacter::OnContact()
{
    if (!IsGrapped)
    {
        return;
    }

    if (IsClient)
    {
        StopFlyOnServer(GetActorLocation());
    }

    StopFly();
}

void ANetCharacter::StopFlyOnServer_Implementation(FVector Location)
{
    SetActorLocation(Location);
    StopFly();
}

void ANetCharacter::StopFly()
{
    IsGrapped = false;
    NetCharacterMovementComponent->SetGrapped(FVector::ZeroVector);
}

void ANetCharacter::ApplyDirectionMarkerToServer_Implementation(FRotator Rotation)
{
    DirectionMarkerRotation = Rotation;
}

void ANetCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	NetCharacterMovementComponent = Cast<UNetCharacterMovementComponent>(Super::GetMovementComponent());
}

void ANetCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANetCharacter, DirectionMarkerRotation);
}

void ANetCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    UpdateDirectionMarker();
}

FVector ANetCharacter::SampleGrappedDestination()
{
    const auto StartLocation = NetCharacterMovementComponent->GetActorLocation();
    const auto EndLocation = StartLocation + Camera->GetActorForwardVector() * FLT_MAX;

    auto HitResult = FHitResult();
    auto TraceParams = FCollisionQueryParams(FName(TEXT("Raycast")), false, GetOwner());
    TraceParams.AddIgnoredComponent(DirectionMarkerView);
    GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams);

    if (HitResult.bBlockingHit)
    {
        const auto HitComponent = HitResult.GetComponent();
        if (HitComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("ANetCharacter::Hit actor: %s"), *HitComponent->GetName());
        }

        DrawDebugSphere(GetWorld(), HitResult.Location, 10.0f, 12, FColor::Green, false, 2.0f, 0, 2.0f);
        //DrawDebugLine(GetWorld(), StartLocation, HitResult.Location, FColor::Green, false, 2.0f, 0, 2.0f);
        return HitResult.Location - StartLocation;
    }
    else
    {
        DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 2.0f);
        return StartLocation;
    }
}

void ANetCharacter::GrappedFly(FVector Destination)
{
    if (IsGrapped)
    {
        return;
    }

    IsGrapped = true;
    NetCharacterMovementComponent->SetGrapped(Destination);
}

bool ANetCharacter::IsLocal() const
{
    return GetWorld()->GetFirstPlayerController()->GetCharacter() == this;
}

void ANetCharacter::UpdateDirectionMarker()
{
    if (IsLocal())
    {
        DirectionMarkerRotation = Camera->GetCameraRotation();

        if (IsClient)
        {
            ApplyDirectionMarkerToServer(DirectionMarkerRotation);
        }
    }

    DirectionMarkerContainer->SetWorldRotation(DirectionMarkerRotation);
}