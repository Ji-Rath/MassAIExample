// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/MassInterpMovementProcessor.h"

#include "FixedTimestep.h"
#include "MassExecutionContext.h"
#include "MassFTSubsystem.h"
#include "Data/FTFragments.h"

UMassInterpMovementProcessor::UMassInterpMovementProcessor() : EntityQuery(*this)
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UMassInterpMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FFTTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UMassFTSubsystem>(EMassFragmentAccess::ReadOnly);
}

void UMassInterpMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		TArrayView<FTransformFragment> TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		TConstArrayView<FFTTransformFragment> FTTransformFragments = Context.GetFragmentView<FFTTransformFragment>();
		const UMassFTSubsystem& FTSubsystem = Context.GetSubsystemChecked<UMassFTSubsystem>();
		
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransformFragment& TransformFragment = TransformFragments[EntityIt];
			const FFTTransformFragment& FTTransformFragment = FTTransformFragments[EntityIt];
			
			const FVector& OldLocation = FTTransformFragment.PreviousLocation;
			const FVector& NewLocation = FTTransformFragment.GetTransform().GetLocation();
			
			FVector InterpLocation = FMath::Lerp(OldLocation, NewLocation, FTSubsystem.GetInterpolationAlpha());
			TransformFragment.GetMutableTransform().SetLocation(InterpLocation);
			
			UE_LOG(LogFixedTimestep, Verbose, TEXT("OldLoc: %s, NewLoc: %s, Alpha: %f"), *OldLocation.ToCompactString(), *NewLocation.ToCompactString(), FTSubsystem.GetInterpolationAlpha())
		}
	});
}
