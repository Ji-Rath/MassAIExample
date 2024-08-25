// Fill out your copyright notice in the Description page of Project Settings.


#include "PersistEntityDataProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassPersistentDataSubsystem.h"
#include "MassSignalSubsystem.h"
#include "PersistentDataFragment.h"

void UPersistEntityDataProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FPersistentDataFragment>(EMassFragmentPresence::All);
	EntityQuery.AddSubsystemRequirement<UMassPersistentDataSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
}

void UPersistEntityDataProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		const auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		const auto PersistentDataFragment = Context.GetConstSharedFragment<FPersistentDataFragment>();
		auto& PersistentDataSubsystem = Context.GetMutableSubsystemChecked<UMassPersistentDataSubsystem>();
		TArray<FEntitySaveData> EntitySaveData;
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const auto& Transform = TransformFragments[EntityIdx];
			FPersistentTransformFragment PersistentTransform;
			PersistentTransform.Transform = Transform.GetTransform();
			
			FEntitySaveData SaveData;
			SaveData.ConfigAsset = PersistentDataFragment.EntityConfig.LoadSynchronous();
			SaveData.EntityFragments = { FInstancedStruct::Make(PersistentTransform) };
			EntitySaveData.Emplace(SaveData);
		}

		PersistentDataSubsystem.FindOrCreateSaveGame()->Entities.Append(EntitySaveData);
	});
	
}

void UPersistEntityDataProcessor::Initialize(UObject& Owner)
{
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, PersistentData::Signals::SaveEntity);
	Super::Initialize(Owner);
}

void UPersistentDataPostLoadProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FPersistentTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FPersistentDataFragment>(EMassFragmentPresence::All);
	EntityQuery.RegisterWithProcessor(*this);
}

void UPersistentDataPostLoadProcessor::SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
	FMassSignalNameLookup& EntitySignals)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		const auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		const auto PersistentTransformFragments = Context.GetFragmentView<FPersistentTransformFragment>();
		TArray<FEntitySaveData> EntitySaveData;
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& Transform = TransformFragments[EntityIdx];
			auto& PersistentTransform = PersistentTransformFragments[EntityIdx];
			
			Transform.GetMutableTransform() = PersistentTransform.Transform;
		}
	});
}

void UPersistentDataPostLoadProcessor::Initialize(UObject& Owner)
{
	UMassSignalSubsystem* SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(Owner.GetWorld());
	SubscribeToSignal(*SignalSubsystem, PersistentData::Signals::EntityLoaded);
	Super::Initialize(Owner);
}


