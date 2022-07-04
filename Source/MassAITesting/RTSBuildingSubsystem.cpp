// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSBuildingSubsystem.h"

#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "RTSItemTrait.h"

void URTSBuildingSubsystem::AddBuilding(const FSmartObjectHandle& BuildingRequest, int Floors)
{
	QueuedBuildings.Emplace(FBuilding(BuildingRequest, Floors));
}

bool URTSBuildingSubsystem::ClaimFloor(FSmartObjectHandle& OutBuilding)
{
	bool bSuccess = false;
	if (QueuedBuildings.Num() > 0)
	{
		FBuilding& BuildStruct = QueuedBuildings[0];
		OutBuilding = BuildStruct.BuildingRequest;
		BuildStruct.FloorsNeeded--;
		//UE_LOG(LogTemp, Error, TEXT("Building ID: %s | Floors remaining: %d"), *LexToString(BuildStruct.BuildingRequest), BuildStruct.FloorsNeeded)
		if (BuildStruct.FloorsNeeded <= 0)
			QueuedBuildings.RemoveAt(0);
		bSuccess = true;
	}
	return bSuccess;
}

bool URTSBuildingSubsystem::FindItem(const FVector& Location, float Radius, EResourceType ResourceType, FMassEntityHandle& OutItemHandle) const
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	TPair<FMassEntityHandle, float> ItemHandle = ItemHashGrid.FindNearestInRadius(Location, Radius, [this, &Location, &EntitySubsystem](const FMassEntityHandle& Handle)
	{
		// Determine distancce
		FVector& OtherLocation = EntitySubsystem->GetFragmentDataPtr<FItemFragment>(Handle)->OldLocation;
		return FVector::Distance(OtherLocation, Location);
	}, [this, &ResourceType, &EntitySubsystem](const FMassEntityHandle& Handle)
	{
		// Determine whether the entity is not claimed and the correct resource
		FItemFragment& Item = EntitySubsystem->GetFragmentDataChecked<FItemFragment>(Handle);
		return Item.bClaimed || Item.ItemType != ResourceType;
	});
	//if (ItemHandle.Key.IsValid())
	//	EntitySubsystem->GetFragmentDataChecked<FItemFragment>(ItemHandle.Key).bClaimed = true;
	OutItemHandle = ItemHandle.Key;
	return ItemHandle.Key.IsValid();
}

bool URTSBuildingSubsystem::ClaimResource(FSmartObjectHandle& OutResourceHandle)
{
	bool bSuccess = false;
	if (QueuedResources.Num() > 0)
	{
		OutResourceHandle = QueuedResources[0];
		//UE_LOG(LogTemp, Error, TEXT("Building ID: %s | Floors remaining: %d"), *LexToString(BuildStruct.BuildingRequest), BuildStruct.FloorsNeeded)
		QueuedResources.RemoveAt(0);
		bSuccess = true;
	}
	return bSuccess;
}

void URTSBuildingSubsystem::AddResourceQueue(FSmartObjectHandle& SOHandle)
{
	QueuedResources.Emplace(SOHandle);
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
