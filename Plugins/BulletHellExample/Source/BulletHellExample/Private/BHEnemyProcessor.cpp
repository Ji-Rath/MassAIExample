// Fill out your copyright notice in the Description page of Project Settings.


#include "BHEnemyProcessor.h"

#include "BHEnemyFragments.h"
#include "BulletHellSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassSimulationLOD.h"
#include "Data/FTFragments.h"

UBHEnemyProcessor::UBHEnemyProcessor()
	: EntityQuery(*this),
	UpdateHashGridQuery(*this)
{
}

void UBHEnemyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FFTMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FBHEnemyTag>(EMassFragmentPresence::All);
	//EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	//EntityQuery.SetChunkFilter(FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
	
	UpdateHashGridQuery.AddSubsystemRequirement<UBulletHellSubsystem>(EMassFragmentAccess::ReadWrite);
	UpdateHashGridQuery.AddRequirement<FBHEnemyFragment>(EMassFragmentAccess::ReadWrite);
	UpdateHashGridQuery.AddRequirement<FFTTransformFragment>(EMassFragmentAccess::ReadOnly);
	//UpdateHashGridQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	//UpdateHashGridQuery.SetChunkFilter(FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame);
}

void UBHEnemyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Update move target
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		SCOPED_NAMED_EVENT(STAT_UpdateMoveTarget, FColor::Red);
		
		TArrayView<FFTMoveTargetFragment> MoveTargetFragments = Context.GetMutableFragmentView<FFTMoveTargetFragment>();
		const UBulletHellSubsystem* BulletHellSubsystem = Context.GetSubsystem<UBulletHellSubsystem>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			FFTMoveTargetFragment& MoveTargetFragment = MoveTargetFragments[EntityIdx];
			
			BulletHellSubsystem->GetPlayerLocation(MoveTargetFragment.Center);
		}
	});

	UpdateHashGridQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		SCOPED_NAMED_EVENT(STAT_UpdateHashGrid, FColor::Red);
		UBulletHellSubsystem* BulletHellSubsystem = Context.GetMutableSubsystem<UBulletHellSubsystem>();
		TArrayView<FBHEnemyFragment> BHEnemyFragments = Context.GetMutableFragmentView<FBHEnemyFragment>();
		TConstArrayView<FFTTransformFragment> TransformFragments = Context.GetFragmentView<FFTTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			FBHEnemyFragment& BHEnemyFragment = BHEnemyFragments[EntityIdx];
			const FFTTransformFragment& TransformFragment = TransformFragments[EntityIdx];

			FVector Location = TransformFragment.GetTransform().GetLocation();
			
			if (BHEnemyFragment.bInitialized)
			{
				BHEnemyFragment.CellLocation = BulletHellSubsystem->GetHashGrid_Mutable().Move(Context.GetEntity(EntityIdx), BHEnemyFragment.CellLocation, FBox::BuildAABB(Location, BHEnemyFragment.CollisionExtent));
			}
		}
	});
}

UBHEnemyInitializer::UBHEnemyInitializer()
	: EntityQuery(*this)
{
	ObservedType = FBHEnemyTag::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Add;
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
			BHEnemyFragment.bInitialized = true;
		}
	});
}

UBHEnemyDestructor::UBHEnemyDestructor()
	: EntityQuery(*this)
{
	ObservedType = FBHEnemyFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Remove;
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
