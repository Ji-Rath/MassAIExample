#pragma once
#include "MassEntityTypes.h"
#include "RandomMovementFragments.generated.h"

namespace RandomMovement
{
	const FName LocationReached = FName(TEXT("LocationReached"));
}

USTRUCT()
struct FRandomMovementFragment : public FMassFragment
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FRandomMovementSettingsFragment : public FMassConstSharedFragment
{
	GENERATED_BODY()

	// Delay between moving to a new random location
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NewLocationDelay = 1.f;

	// Radius to find a new random position
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float NewLocationRadius = 500.f;
};
