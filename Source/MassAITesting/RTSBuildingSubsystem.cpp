// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSBuildingSubsystem.h"

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
		UE_LOG(LogTemp, Error, TEXT("Building ID: %s | Floors remaining: %d"), *LexToString(BuildStruct.BuildingRequest), BuildStruct.FloorsNeeded)
		if (BuildStruct.FloorsNeeded <= 0)
			QueuedBuildings.RemoveAt(0);
	}
}