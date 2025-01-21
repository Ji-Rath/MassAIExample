// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchGridHeightProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassAITesting/Subsystems/GridManagerSubsystem.h"

void UMatchGridHeightProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UGridManagerSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMatchGridHeightProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto& GridManagerSubsystem = Context.GetMutableSubsystemChecked<UGridManagerSubsystem>();
		
		const auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& TransformFragment = TransformFragments[EntityIdx];
			FVector NodeLocation = GridManagerSubsystem.GetClosestNode(TransformFragment.GetTransform().GetLocation());
			if (NodeLocation == FVector::ZeroVector) { continue; } // Invalid node location

			// Align entity to grid
			NodeLocation.X = TransformFragment.GetTransform().GetLocation().X;
			NodeLocation.Y = TransformFragment.GetTransform().GetLocation().Y;
			TransformFragment.GetMutableTransform().SetLocation(NodeLocation);
		}
	});
}
