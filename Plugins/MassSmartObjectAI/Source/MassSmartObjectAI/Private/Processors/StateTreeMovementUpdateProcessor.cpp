// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/StateTreeMovementUpdateProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeFragments.h"

UStateTreeMovementUpdateProcessor::UStateTreeMovementUpdateProcessor()
{
}

void UStateTreeMovementUpdateProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassStateTreeInstanceFragment>(EMassFragmentAccess::None);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UMassSignalSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UStateTreeMovementUpdateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto& SignalSubsystem = Context.GetMutableSubsystemChecked<UMassSignalSubsystem>();
		
		const auto MassMoveTargetFragments = Context.GetFragmentView<FMassMoveTargetFragment>();
		const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const auto& MassMoveTargetFragment = MassMoveTargetFragments[EntityIdx];
			const auto& TransformFragment = TransformFragments[EntityIdx];
			
			if (MassMoveTargetFragment.GetCurrentAction() == EMassMovementAction::Move)
			{
				auto Distance = FVector::Dist2D(TransformFragment.GetTransform().GetLocation(), MassMoveTargetFragment.Center);
				if (Distance < 100.f)
				{
					SignalSubsystem.SignalEntityDeferred(Context, UE::Mass::Signals::StateTreeActivate, Context.GetEntity(EntityIdx));	
				}
			}
		}
	});
}
