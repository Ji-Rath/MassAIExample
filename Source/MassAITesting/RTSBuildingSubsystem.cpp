// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSBuildingSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "RTSItemTrait.h"

void URTSBuildingSubsystem::AddBuilding(const FSmartObjectHandle& BuildingRequest, int Floors)
{
	QueuedBuildings.Emplace(FBuilding(BuildingRequest, Floors));
}

void URTSBuildingSubsystem::ClaimFloor(FSmartObjectHandle& Building)
{
	if (QueuedBuildings.Num() > 0)
	{
		FBuilding& BuildStruct = QueuedBuildings[0];
		Building = BuildStruct.BuildingRequest;
		BuildStruct.FloorsNeeded--;
		//UE_LOG(LogTemp, Error, TEXT("Building ID: %s | Floors remaining: %d"), *LexToString(BuildStruct.BuildingRequest), BuildStruct.FloorsNeeded)
		if (BuildStruct.FloorsNeeded <= 0)
			QueuedBuildings.RemoveAt(0);
	}
}

FMassEntityHandle URTSBuildingSubsystem::FindItem(FVector2D Location, float Radius, EResourceType ResourceType) const
{
	TPair<FMassEntityHandle, float> ItemHandle = ItemHashGrid.FindNearestInRadius(Location, Radius, [this, &Location](const FMassEntityHandle& Handle)
	{
		// Determine distancce
		FVector2D& OtherLocation = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetFragmentDataPtr<FItemFragment>(Handle)->OldLocation;
		return FVector2D::Distance(OtherLocation, Location);
	}, [this, &ResourceType](const FMassEntityHandle& Handle)
	{
		// Determine whether the entity is not claimed and the correct resource
		FItemFragment& Item = GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetFragmentDataChecked<FItemFragment>(Handle);
		return Item.bClaimed || Item.ItemType != ResourceType;
	});
	return ItemHandle.Key;
}

void URTSBuildingSubsystem::AddRTSAgent(const FMassEntityHandle& Entity)
{
	RTSAgents.Emplace(Entity);
}

void URTSBuildingSubsystem::SelectClosestAgent(const FVector& Location)
{
	float ClosestDistance = -1;
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	for(const FMassEntityHandle& Entity : RTSAgents)
	{
		const FVector& EntityLocation = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(Entity)->GetTransform().GetLocation();
		float Distance = FVector::Dist(EntityLocation, Location);
		if (ClosestDistance == -1 || Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			RTSAgent = Entity;
		}
	}
}

void URTSBuildingSubsystem::GetAgentLocation(FVector& OutLocation)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (EntitySubsystem && RTSAgent.IsValid())
	{
		OutLocation = EntitySubsystem->GetFragmentDataPtr<FTransformFragment>(RTSAgent)->GetTransform().GetLocation();
	}
}

void URTSBuildingSubsystem::GetAgentInformation(FRTSAgentFragment& OutAgentInfo)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (EntitySubsystem && RTSAgent.IsValid())
	{
		OutAgentInfo = *EntitySubsystem->GetFragmentDataPtr<FRTSAgentFragment>(RTSAgent);
	}
}
