// Fill out your copyright notice in the Description page of Project Settings.


#include "BHEnemyProcessor.h"

#include "BHEnemyFragments.h"
#include "BulletHellSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassSimulationLOD.h"

UBHEnemyProcessor::UBHEnemyProcessor()
	: EntityQuery(*this),
	UpdateHashGridQuery(*this)
{
}

void UBHEnemyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FBHEnemyTag>(EMassFragmentPresence::All);
	
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter(FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
	
	UpdateHashGridQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadWrite);
	UpdateHashGridQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	UpdateHashGridQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);

	UpdateHashGridQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	UpdateHashGridQuery.SetChunkFilter(FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UBHEnemyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Update move target
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		SCOPED_NAMED_EVENT(STAT_UpdateMoveTarget, FColor::Red);
		auto BulletHellSubsystem = Context.GetSubsystem<UBulletHellSubsystem>();
		auto MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& MoveTargetFragment = MoveTargetFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];
			
			BulletHellSubsystem->GetPlayerLocation(MoveTargetFragment.Center);

			auto EntityLocation = TransformFragment.GetTransform().GetLocation();
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

	UpdateHashGridQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		SCOPED_NAMED_EVENT(STAT_UpdateHashGrid, FColor::Red);
		auto BulletHellSubsystem = Context.GetMutableSubsystem<UBulletHellSubsystem>();
		auto BHEnemyFragments = Context.GetMutableFragmentView<FBHEnemyFragment>();
		auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto TransformFragment = TransformFragments[EntityIdx];
			auto& BHEnemyFragment = BHEnemyFragments[EntityIdx];

			auto Location = TransformFragment.GetTransform().GetLocation();

			BHEnemyFragment.CellLocation = BulletHellSubsystem->GetHashGrid_Mutable().Move(Context.GetEntity(EntityIdx), BHEnemyFragment.CellLocation, FBox::BuildAABB(Location, BHEnemyFragment.CollisionExtent));
		}
	});
}

UBHEnemyInitializer::UBHEnemyInitializer()
	: EntityQuery(*this)
{
	ObservedType = FBHEnemyTag::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UBHEnemyInitializer::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UBHEnemyInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
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
	: EntityQuery(*this)
{
	ObservedType = FBHEnemyFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void UBHEnemyDestructor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UBHEnemyDestructor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
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
