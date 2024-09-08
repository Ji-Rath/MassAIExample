// Fill out your copyright notice in the Description page of Project Settings.


#include "HashGridSubsystem.h"

#include "HashGridFragments.h"
#include "MassEntityUtils.h"
#include "MassSignalSubsystem.h"

void UHashGridSubsystem::SelectEntitiesInArea(const FVector& SelectedLocation, float Radius)
{
	FBox Bounds = { SelectedLocation - Radius, SelectedLocation + Radius };
	TArray<FMassEntityHandle> EntitiesQueried;
	HashGridData.Query(Bounds, EntitiesQueried);

	auto EntityManager = UE::Mass::Utils::GetEntityManager(GetWorld());
	auto EntitySignalSubsystem = GetWorld()->GetSubsystem<UMassSignalSubsystem>();
	if (EntitiesQueried.IsEmpty()) { return; } // Its ok if nothing was queried
	
	EntitySignalSubsystem->SignalEntities(HashGridExample::Signals::EntityQueried, EntitiesQueried);
}
