// Fill out your copyright notice in the Description page of Project Settings.


#include "TickSTProcessor.h"

#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeFragments.h"

void UTickSTProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassStateTreeInstanceFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UTickSTProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{

	// Iterate through chunks
	EntityQuery.ForEachEntityChunk(EntityManager, Context,[](FMassExecutionContext& Context)
	{
		UMassSignalSubsystem& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
		const auto& StateTreeInstanceArray = Context.GetFragmentView<FMassStateTreeInstanceFragment>();
		
		// Iterate through entities in chunk
		const int32 NumEntities = Context.GetNumEntities();
		for(int32 EntityIndex = 0; EntityIndex < NumEntities; EntityIndex++)
		{
			const auto& StateTreeInstance = StateTreeInstanceArray[EntityIndex];
			
			SignalSubsystem.DelaySignalEntityDeferred(Context, UE::Mass::Signals::StateTreeActivate, Context.GetEntity(EntityIndex), 1.f);
		}
	});
}
