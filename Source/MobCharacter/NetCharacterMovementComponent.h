#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NetCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MOBCHARACTER_API UNetCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:

	UNetCharacterMovementComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GrappedSpeed = 1000;

	void SetGrapped(FVector Direction);
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	float InitialGravityScale = 0;
    FVector GrappedDirection;
};
