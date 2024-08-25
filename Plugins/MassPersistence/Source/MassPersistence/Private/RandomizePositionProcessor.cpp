// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomizePositionProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassPersistentDataSubsystem.h"

void URandomizePositionProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
	Super::ConfigureQueries();
}

void URandomizePositionProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		const auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& TransformFragment = TransformFragments[EntityIdx];
			TransformFragment.GetMutableTransform().SetLocation(FVector(FMath::RandRange(-2000, 2000), FMath::RandRange(-2000, 2000), 0.f));
		}
	});
}

void URandomizePositionProcessor::Initialize(UObject& Owner)
{
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, PersistentData::Signals::RandomizePositions);
	Super::Initialize(Owner);
}
