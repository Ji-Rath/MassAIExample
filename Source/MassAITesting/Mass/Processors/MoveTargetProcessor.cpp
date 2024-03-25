// Fill out your copyright notice in the Description page of Project Settings.


#include "MoveTargetProcessor.h"

#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassSignalSubsystem.h"
#include "MassSimulationLOD.h"
#include "MassAITesting/StateTree/Tasks/MassSetSmartObjectMoveTargetTask.h"

UMoveTargetProcessor::UMoveTargetProcessor()
{
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UMoveTargetProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter(&FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UMoveTargetProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	TArray<FMassEntityHandle> EntitiesToSignal;
	EntityQuery.ForEachEntityChunk(EntityManager, Context, ([this, &EntitiesToSignal](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> TransformsList = Context.GetMutableFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			FTransform& Transform = TransformsList[EntityIndex].GetMutableTransform();
			FMassMoveTargetFragment& MoveTarget = MoveTargets[EntityIndex];

			if (MoveTarget.GetCurrentAction() == EMassMovementAction::Move)
			{
				MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
				MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();

				if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
				{
					EntitiesToSignal.Add(Context.GetEntity(EntityIndex));
				}
			}
		}
	}));

	if (EntitiesToSignal.Num())
	{
		SignalSubsystem->SignalEntities(UE::Mass::Signals::FollowPointPathDone, EntitiesToSignal);
	}
}

void UMoveTargetProcessor::Initialize(UObject& Owner)
{
	SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(GetWorld());
}
