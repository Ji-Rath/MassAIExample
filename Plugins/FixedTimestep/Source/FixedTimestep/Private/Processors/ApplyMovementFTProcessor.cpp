// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/ApplyMovementFTProcessor.h"

#include "MassExecutionContext.h"
#include "Data/FTFragments.h"

UApplyMovementFTProcessor::UApplyMovementFTProcessor() : EntityQuery(*this)
{
}

void UApplyMovementFTProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FFTTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FFTVelocityFragment>(EMassFragmentAccess::ReadOnly);
}

void UApplyMovementFTProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ApplyMovementProcessor)
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		TArrayView<FFTTransformFragment> TransformFragments = Context.GetMutableFragmentView<FFTTransformFragment>();
		TConstArrayView<FFTVelocityFragment> VelocityFragments = Context.GetFragmentView<FFTVelocityFragment>();
		
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FFTTransformFragment& TransformFragment = TransformFragments[EntityIt];
			const FFTVelocityFragment& VelocityFragment = VelocityFragments[EntityIt];
			
			FVector CurrentLocation = TransformFragment.GetMutableTransform().GetLocation();
			
			TransformFragment.PreviousLocation = CurrentLocation;
			TransformFragment.GetMutableTransform().SetLocation(CurrentLocation + (VelocityFragment.Value * Context.GetDeltaTimeSeconds()));
		}
	});
}
