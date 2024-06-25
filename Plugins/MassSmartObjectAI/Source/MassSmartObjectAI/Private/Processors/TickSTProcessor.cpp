// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/TickSTProcessor.h"

#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeFragments.h"

void UTickSTProcessor::ConfigureQueries()
{
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UTickSTProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Iterate through chunks
	EntityQuery.ForEachEntityChunk(EntityManager, Context,[](FMassExecutionContext& Context)
	{
		UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
		
		// Iterate through entities in chunk
		const int32 NumEntities = Context.GetNumEntities();
		for(int32 EntityIndex = 0; EntityIndex < NumEntities; EntityIndex++)
		{
			// This will signal the state tree to tick (evaluators, global tasks, etc)
			// In the real world, this would be throttled rather than being run every tick (LOD, etc)
			// To go a level deeper, we would only tick when needed (event based)
			SignalSubsystem.SignalEntityDeferred(Context, UE::Mass::Signals::StateTreeActivate, Context.GetEntity(EntityIndex));
		}
	});
}
