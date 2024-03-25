#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NetCharacterMovementComponent.h"

#include "NetCharacter.generated.h"

UCLASS()
class MOBCHARACTER_API ANetCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANetCharacter(const FObjectInitializer& ObjectInitializer);

protected:

	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable, Category = "Movement")
	FORCEINLINE class UNetCharacterMovementComponent* GetMyMovementComponent() const { return NetCharacterMovementComponent; }

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Move(float x, float y);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Look(float x, float y);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Grapped();

	UFUNCTION(Server, Reliable)
	void GrappedFlyOnServer(FVector Destination);

	UFUNCTION(Client, Reliable)
	void GrappedFlyOnClient(FVector Destination);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void OnContact();

	UFUNCTION(Server, Reliable)
	void StopFlyOnServer(FVector Location);

	void StopFly();

	UPROPERTY(Replicated);
	FRotator DirectionMarkerRotation;

	UFUNCTION(Server, Unreliable)
	void ApplyDirectionMarkerToServer(FRotator Rotation);

	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaTime) override;

private:

	APlayerCameraManager* Camera;
	UStaticMeshComponent* DirectionMarkerContainer;
	UStaticMeshComponent* DirectionMarkerView;
	UNetCharacterMovementComponent* NetCharacterMovementComponent;
	bool IsGrapped = false;
	bool IsServer = false;
	bool IsClient = false;

	FVector SampleGrappedDestination();
	void GrappedFly(FVector Destination);
	bool IsLocal() const;
	void UpdateDirectionMarker();
};