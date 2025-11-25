// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletHellSubsystem.h"

#include "BulletFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySubsystem.h"
#include "MassSignalSubsystem.h"
#include "MassSpawnerSubsystem.h"

const FBHEntityHashGrid& UBulletHellSubsystem::GetHashGrid() const
{
	return EntityHashGrid;
}

FBHEntityHashGrid& UBulletHellSubsystem::GetHashGrid_Mutable()
{
	return EntityHashGrid;
}

void UBulletHellSubsystem::GetPlayerLocation(FVector& OutLocation) const
{
	OutLocation = PlayerLocation;
}

void UBulletHellSubsystem::SpawnBullet(UMassEntityConfigAsset* BulletConfig, const FVector& Location, const FVector& Direction)
{
	check(BulletConfig);

	auto SignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
	auto SpawnerSystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
	auto& EntityManager = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetMutableEntityManager();
	
	TArray<FMassEntityHandle> EntitiesSpawned;
	SpawnerSystem->SpawnEntities(BulletConfig->GetOrCreateEntityTemplate(*GetWorld()), 1, EntitiesSpawned);
	
	auto& BulletFragment = EntityManager.GetFragmentDataChecked<FBulletFragment>(EntitiesSpawned[0]);
	BulletFragment.Direction = Direction;
	BulletFragment.SpawnLocation = Location;
	
	SignalSubsystem->SignalEntity(BulletHell::Signals::BulletSpawned, EntitiesSpawned[0]);
}

void UBulletHellSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	if (InWorld.GetFirstPlayerController())
	{
		CachedPlayerPawn = InWorld.GetFirstPlayerController()->GetPawn();
	}
}

TStatId UBulletHellSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UBulletHellSubsystem, STATGROUP_Tickables);
}

void UBulletHellSubsystem::Tick(float DeltaTime)
{
	if (CachedPlayerPawn)
	{
		PlayerLocation = CachedPlayerPawn->GetActorLocation();
	}
}
