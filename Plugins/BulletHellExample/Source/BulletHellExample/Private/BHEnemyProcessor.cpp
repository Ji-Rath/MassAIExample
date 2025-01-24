// Fill out your copyright notice in the Description page of Project Settings.


#include "BHEnemyProcessor.h"

#include "BHEnemyFragments.h"
#include "BulletHellSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"

void UBHEnemyProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FBHEnemyTag>(EMassFragmentPresence::All);
	EntityQuery.RegisterWithProcessor(*this);
}

void UBHEnemyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Update move target
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto BulletHellSubsystem = Context.GetSubsystem<UBulletHellSubsystem>();
		auto MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& MoveTargetFragment = MoveTargetFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];
			BulletHellSubsystem->GetPlayerLocation(MoveTargetFragment.Center);

			FVector EntityLocation = TransformFragment.GetTransform().GetLocation();
			MoveTargetFragment.DistanceToGoal = FVector::Dist(EntityLocation, MoveTargetFragment.Center);
			MoveTargetFragment.Forward = (MoveTargetFragment.Center - EntityLocation).GetSafeNormal();

			// Update action based on distance to goal
			if (MoveTargetFragment.GetCurrentAction() == EMassMovementAction::Stand && MoveTargetFragment.DistanceToGoal > 50.f)
			{
				MoveTargetFragment.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
				MoveTargetFragment.IntentAtGoal = EMassMovementAction::Stand;
			}
			else if (MoveTargetFragment.GetCurrentAction() == EMassMovementAction::Move && MoveTargetFragment.DistanceToGoal <= 50.f)
			{
				MoveTargetFragment.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
			}
		}
	});
}
