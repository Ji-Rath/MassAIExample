// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomizePositionProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassSignalSubsystem.h"
#include "MassPersistentDataSubsystem.h"

URandomizePositionProcessor::URandomizePositionProcessor()
	: EntityQuery(*this)
{
}

void URandomizePositionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void URandomizePositionProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
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

void URandomizePositionProcessor::InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager)
{
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, PersistentData::Signals::RandomizePositions);
	Super::InitializeInternal(Owner, EntityManager);
}
