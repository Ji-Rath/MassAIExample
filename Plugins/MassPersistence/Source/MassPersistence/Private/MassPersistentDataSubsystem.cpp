// Fill out your copyright notice in the Description page of Project Settings.


#include "MassPersistentDataSubsystem.h"

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
	GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntities(PersistentData::Signals::SaveEntity, Entities);
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
	auto SpawnerSystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
	auto& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	for (const auto& EntityData : SaveGameFile->Entities)
	{
		if (!EntityData.ConfigAsset) { continue; }
		TArray<FMassEntityHandle> EntitiesSpawned;
		SpawnerSystem->SpawnEntities(EntityData.ConfigAsset->GetOrCreateEntityTemplate(*GetWorld()), 1, EntitiesSpawned);
		EntityManager.SetEntityFragmentsValues(EntitiesSpawned[0], EntityData.EntityFragments);
		AllEntitiesLoaded.Emplace(EntitiesSpawned[0]);
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
	GetWorld()->GetSubsystem<UMassSignalSubsystem>()->SignalEntities(PersistentData::Signals::RandomizePositions, ManagedEntities);
}

void UMassPersistentDataSubsystem::ClearPersistedEntities()
{
	if (ManagedEntities.IsEmpty()) { return; }
	GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager().BatchDestroyEntities(ManagedEntities);
	ManagedEntities.Empty();
}
