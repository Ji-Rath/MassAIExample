// Fill out your copyright notice in the Description page of Project Settings.


#include "PersistentDataSetupProcessor.h"

#include "MassExecutionContext.h"
#include "MassPersistentDataSubsystem.h"
#include "PersistentDataFragment.h"

UPersistentDataInitializerProcessor::UPersistentDataInitializerProcessor() :
	EntityQuery(*this)
{
	ObservedType = FPersistentDataTag::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UPersistentDataInitializerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddSubsystemRequirement<UMassPersistentDataSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UPersistentDataInitializerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		auto& PersistentDataSubsystem = Context.GetMutableSubsystemChecked<UMassPersistentDataSubsystem>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			PersistentDataSubsystem.ManagedEntities.Emplace(Context.GetEntity(EntityIdx));
		}
	});
}

UPersistentDataDestructorProcessor::UPersistentDataDestructorProcessor() :
	EntityQuery(*this)
{
	ObservedType = FPersistentDataTag::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void UPersistentDataDestructorProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddSubsystemRequirement<UMassPersistentDataSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UPersistentDataDestructorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		auto& PersistentDataSubsystem = Context.GetMutableSubsystemChecked<UMassPersistentDataSubsystem>();

		TSet<FMassEntityHandle> EntitiesToRemove;
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			EntitiesToRemove.Add(Context.GetEntity(EntityIdx));
		}
		
		PersistentDataSubsystem.ManagedEntities.RemoveAllSwap([&EntitiesToRemove](FMassEntityHandle& Entity)
		{
			return EntitiesToRemove.Contains(Entity);
		});
	});
	
}