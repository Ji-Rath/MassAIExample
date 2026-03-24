#pragma once

#include "CoreMinimal.h"
#include "MassCommonFragments.h"
#include "MassEntityTraitBase.h"
#include "MassMovementFragments.h"
#include "FTFragments.generated.h"

namespace UE::Mass::ProcessorGroupNames
{
	const FName FixedTimestep = FName(TEXT("FixedTimestep"));
}

// The real transform of the entity while using a fixed time step. This will get sent to a translator to update the 'visual' FTransformFragment
USTRUCT()
struct FFTTransformFragment : public FTransformFragment
{
	GENERATED_BODY()
	
	// Stores the last frame location of the entity 
	UPROPERTY()
	FVector PreviousLocation;
};

// The velocity of the entity while using a fixed time step.
USTRUCT()
struct FFTVelocityFragment : public FMassVelocityFragment
{
	GENERATED_BODY()
};

// The desired velocity for this entity. Movement processors will write to this fragment
USTRUCT()
struct FFTDesiredMovementFragment : public FMassVelocityFragment
{
	GENERATED_BODY()
};

UCLASS()
class UFTMovementTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

public:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
