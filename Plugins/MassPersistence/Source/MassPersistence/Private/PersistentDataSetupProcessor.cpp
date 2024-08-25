// Fill out your copyright notice in the Description page of Project Settings.


#include "PersistentDataSetupProcessor.h"

#include "MassExecutionContext.h"
#include "MassPersistentDataSubsystem.h"
#include "PersistentDataFragment.h"

void UPersistentDataInitializerProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

UPersistentDataInitializerProcessor::UPersistentDataInitializerProcessor()
{
	ObservedType = FPersistentDataTag::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UPersistentDataInitializerProcessor::ConfigureQueries()
{
	EntityQuery.AddSubsystemRequirement<UMassPersistentDataSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UPersistentDataInitializerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto& PersistentDataSubsystem = Context.GetMutableSubsystemChecked<UMassPersistentDataSubsystem>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			PersistentDataSubsystem.ManagedEntities.Emplace(Context.GetEntity(EntityIdx));
		}
	});
}

void UPersistentDataDestructorProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

UPersistentDataDestructorProcessor::UPersistentDataDestructorProcessor()
{
	ObservedType = FPersistentDataTag::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void UPersistentDataDestructorProcessor::ConfigureQueries()
{
	EntityQuery.AddSubsystemRequirement<UMassPersistentDataSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UPersistentDataDestructorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
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