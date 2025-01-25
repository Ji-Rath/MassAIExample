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

	UpdateHashGridQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadWrite);
	UpdateHashGridQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	UpdateHashGridQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);
	UpdateHashGridQuery.RegisterWithProcessor(*this);
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

	UpdateHashGridQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto BulletHellSubsystem = Context.GetMutableSubsystem<UBulletHellSubsystem>();
		auto BHEnemyFragments = Context.GetMutableFragmentView<FBHEnemyFragment>();
		auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto TransformFragment = TransformFragments[EntityIdx];
			auto BHEnemyFragment = BHEnemyFragments[EntityIdx];

			auto Location = TransformFragment.GetTransform().GetLocation();

			BHEnemyFragment.CellLocation = BulletHellSubsystem->GetHashGrid_Mutable().Move(Context.GetEntity(EntityIdx), BHEnemyFragment.CellLocation, FBox::BuildAABB(Location, BHEnemyFragment.CollisionExtent));
		}
	});
}

UBHEnemyInitializer::UBHEnemyInitializer()
{
	ObservedType = FBHEnemyTag::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UBHEnemyInitializer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UBHEnemyInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		auto BHEnemyFragments = Context.GetMutableFragmentView<FBHEnemyFragment>();
		auto BulletHellSubsystem = Context.GetMutableSubsystem<UBulletHellSubsystem>();
		auto& HashGrid = BulletHellSubsystem->GetHashGrid_Mutable();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& BHEnemyFragment = BHEnemyFragments[EntityIdx];
			auto TransformFragment = TransformFragments[EntityIdx];
			auto Location = TransformFragment.GetTransform().GetLocation();
			
			BHEnemyFragment.CellLocation = HashGrid.Add(Context.GetEntity(EntityIdx), FBox::BuildAABB(Location, BHEnemyFragment.CollisionExtent));
		}
	});
}

UBHEnemyDestructor::UBHEnemyDestructor()
{
	ObservedType = FBHEnemyFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void UBHEnemyDestructor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UBHEnemyDestructor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto BHEnemyFragments = Context.GetFragmentView<FBHEnemyFragment>();
		auto BulletHellSubsystem = Context.GetMutableSubsystem<UBulletHellSubsystem>();
		auto& HashGrid = BulletHellSubsystem->GetHashGrid_Mutable();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& BHEnemyFragment = BHEnemyFragments[EntityIdx];
			
			HashGrid.Remove(Context.GetEntity(EntityIdx), BHEnemyFragment.CellLocation);
		}
	});
}
