// Fill out your copyright notice in the Description page of Project Settings.


#include "MassPersistentDataSubsystem.h"

#include "MassEntityBuilder.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySubsystem.h"
#include "MassSaveGame.h"
#include "MassSignalSubsystem.h"
#include "MassSpawnerSubsystem.h"

void UMassPersistentDataSubsystem::SaveEntities(const FString& SlotName)
{
	SaveEntityData(ManagedEntities);
}

void UMassPersistentDataSubsystem::SaveEntityData(TArray<FMassEntityHandle>& Entities)
{
	if (ManagedEntities.IsEmpty()) { return; }
	
	auto SignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
	SignalSubsystem->SignalEntities(PersistentData::Signals::SaveEntity, Entities);
}

UMassSaveGame* UMassPersistentDataSubsystem::FindOrCreateSaveGame()
{
	if (!SaveGame) { SaveGame = NewObject<UMassSaveGame>(GetTransientPackage()); }
	return SaveGame;
}

void UMassPersistentDataSubsystem::LoadEntitiesFromSave(UMassSaveGame* SaveGameFile)
{
	ClearPersistedEntities();
	
	TArray<FMassEntityHandle> AllEntitiesLoaded;
	auto SignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
	auto& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	
	for (const auto& EntityData : SaveGameFile->Entities)
	{
		if (!ensure(EntityData.ConfigAsset)) { continue; }

		// create entity based on template
		const auto& Template = EntityData.ConfigAsset->GetOrCreateEntityTemplate(*GetWorld());
		auto Entity = Template.CreateEntityBuilder(EntityManager.AsShared());

		// Use fragment data from save
		for (const FInstancedStruct& EntityFragment : EntityData.EntityFragments)
		{
			Entity.Add(EntityFragment);
		}
		
		AllEntitiesLoaded.Emplace(Entity.Commit());
	}
	
	SignalSubsystem->SignalEntities(PersistentData::Signals::EntityLoaded, AllEntitiesLoaded);
}

void UMassPersistentDataSubsystem::SpawnEntities(UMassEntityConfigAsset* ConfigAsset, int Amount)
{
	auto SpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
	TArray<FMassEntityHandle> Entities;
	SpawnerSubsystem->SpawnEntities(ConfigAsset->GetOrCreateEntityTemplate(*GetWorld()), Amount, Entities);
}

void UMassPersistentDataSubsystem::RandomizePositions()
{
	if (ManagedEntities.IsEmpty()) { return; }
	auto SignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
	SignalSubsystem->SignalEntities(PersistentData::Signals::RandomizePositions, ManagedEntities);
}

void UMassPersistentDataSubsystem::ClearPersistedEntities()
{
	if (ManagedEntities.IsEmpty()) { return; }
	
	auto& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	EntityManager.Defer().DestroyEntities(ManagedEntities);
	
	ManagedEntities.Empty();
}
