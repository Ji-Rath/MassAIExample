// Fill out your copyright notice in the Description page of Project Settings.


#include "HashGridExampleProcessors.h"

#include "HashGridFragments.h"
#include "HashGridSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"

static float HalfRange = 25.f;

UHashGridInitializeProcessor::UHashGridInitializeProcessor()
{
	ObservedType = FHashGridFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void UHashGridInitializeProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FHashGridFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UHashGridSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UHashGridInitializeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UHashGridSubsystem>();
		
		const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const auto HashGridFragments = Context.GetMutableFragmentView<FHashGridFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& HashGridFragment = HashGridFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];
			auto Location = TransformFragment.GetTransform().GetLocation();
			
			FBox Bounds = { Location - HalfRange, Location + HalfRange };
			HashGridFragment.CellLocation = HashGridSubsystem.HashGridData.Add(Context.GetEntity(EntityIdx), Bounds);
		}
	});
}

UHashGridDestroyProcessor::UHashGridDestroyProcessor()
{
	ObservedType = FHashGridFragment::StaticStruct();
	Operation = EMassObservedOperation::Remove;
}

void UHashGridDestroyProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHashGridFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UHashGridSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UHashGridDestroyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UHashGridSubsystem>();
		
		const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const auto HashGridFragments = Context.GetMutableFragmentView<FHashGridFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& HashGridFragment = HashGridFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];

			HashGridSubsystem.HashGridData.Remove(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation);
		}
	});
}

void UHashGridProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FHashGridFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UHashGridSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UHashGridProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		auto& HashGridSubsystem = Context.GetMutableSubsystemChecked<UHashGridSubsystem>();
		
		const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const auto HashGridFragments = Context.GetMutableFragmentView<FHashGridFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& HashGridFragment = HashGridFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];
			const auto& Location = TransformFragment.GetTransform().GetLocation();

			// Move the entity to the new location
			FBox Bounds = { Location - HalfRange, Location + HalfRange };
			auto NewCellLocation = HashGridSubsystem.HashGridData.Move(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation, Bounds);
			HashGridFragment.CellLocation = NewCellLocation;
		}
	});
}

void UHashGridQueryProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, HashGridExample::Signals::EntityQueried);
}

void UHashGridQueryProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FHashGridFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

void UHashGridQueryProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
	{
		const auto HashGridFragments = Context.GetMutableFragmentView<FHashGridFragment>();
		const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& HashGridFragment = HashGridFragments[EntityIdx];
			auto& TransformFragment = TransformFragments[EntityIdx];

			// Destroy the entity when we receive the signal
			EntityManager.Defer().DestroyEntity(Context.GetEntity(EntityIdx));

			DrawDebugPoint(GetWorld(), TransformFragment.GetTransform().GetLocation(), 50.f, FColor::Yellow, true);
		}
	});
}
