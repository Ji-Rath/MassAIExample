// Fill out your copyright notice in the Description page of Project Settings.


#include "Mass/Animation/VertexAnimLocomotionProcessor.h"

#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "Mass/Animation/VertexAnimProcessor.h"

UVertexAnimLocomotionProcessor::UVertexAnimLocomotionProcessor()
{
}

void UVertexAnimLocomotionProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FVertexAnimInfoFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FVertexAnimLocomotionFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

void UVertexAnimLocomotionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		const auto MassVelocityFragments = Context.GetFragmentView<FMassVelocityFragment>();
		const auto VertexAnimFragments = Context.GetMutableFragmentView<FVertexAnimInfoFragment>();
		const auto VertexAnimLocomotionFragments = Context.GetFragmentView<FVertexAnimLocomotionFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const auto& VelocityFragment = MassVelocityFragments[EntityIdx];
			auto& VertexAnimFragment = VertexAnimFragments[EntityIdx];
			const auto& VertexAnimLocomotionFragment = VertexAnimLocomotionFragments[EntityIdx];
			
			float Speed = VelocityFragment.Value.Length();
			VertexAnimFragment.AnimationStateIndex = Speed >= VertexAnimLocomotionFragment.SpeedThreshhold ? VertexAnimLocomotionFragment.RunAnimIndex : VertexAnimLocomotionFragment.IdleAnimIndex;
		}
	});
}
