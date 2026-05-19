// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/DesiredMovementProcessor.h"

#include "MassExecutionContext.h"
#include "Data/FTFragments.h"
#include "Processors/ApplyMovementFTProcessor.h"

UDesiredMovementProcessor::UDesiredMovementProcessor() : EntityQuery(*this)
{
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
	ExecutionOrder.ExecuteBefore.Add(UApplyMovementFTProcessor::StaticClass()->GetFName());
}

void UDesiredMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FFTDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FFTMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FFTTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FMovementSettings>();
}

void UDesiredMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		TArrayView<FFTDesiredMovementFragment> DesiredMovementFragments = Context.GetMutableFragmentView<FFTDesiredMovementFragment>();
		TConstArrayView<FFTMoveTargetFragment> MoveTargetFragments = Context.GetFragmentView<FFTMoveTargetFragment>();
		TConstArrayView<FFTTransformFragment> TransformFragments = Context.GetFragmentView<FFTTransformFragment>();
		
		const FMovementSettings& MovementSettings = Context.GetConstSharedFragment<FMovementSettings>();
		
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FFTDesiredMovementFragment& DesiredMovementFragment = DesiredMovementFragments[EntityIt];
			const FFTMoveTargetFragment& MoveTargetFragment = MoveTargetFragments[EntityIt];
			const FFTTransformFragment& TransformFragment = TransformFragments[EntityIt];
			
			const FVector& Location = TransformFragment.GetTransform().GetLocation();
			const FVector& DesiredLocation = MoveTargetFragment.Center;
			
			DesiredMovementFragment.DesiredVelocity = (DesiredLocation - Location).GetClampedToMaxSize(MovementSettings.MaxSpeed);
		}
	});
}
