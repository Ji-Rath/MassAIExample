// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"

void UCollisionSubsystem::SpawnEntities(const FVector& Location, int Count, UMassEntityConfigAsset* EntityConfig)
{
	auto SpawnerSystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
	auto& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	
	TArray<FMassEntityHandle> EntitiesSpawned;
	SpawnerSystem->SpawnEntities(EntityConfig->GetOrCreateEntityTemplate(*GetWorld()), Count, EntitiesSpawned);

	for (auto& Entity : EntitiesSpawned)
	{
		auto& TransformFragment = EntityManager.GetFragmentDataChecked<FTransformFragment>(Entity);
		TransformFragment.GetMutableTransform().SetLocation(Location);
	}
}
