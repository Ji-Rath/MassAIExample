// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSAgentTrait.h"
#include "SmartObjectSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "RTSBuildingSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateInventory, FMassEntityHandle, Entity);

USTRUCT()
struct MASSAITESTING_API FBuilding
{
	GENERATED_BODY()
	
	UPROPERTY()
	FSmartObjectHandle BuildingRequest;

	UPROPERTY()
	int FloorsNeeded = 1;

	FBuilding() {}

	FBuilding(const FSmartObjectHandle& BuildingRequest, int FloorsNeeded)
	{
		this->BuildingRequest = BuildingRequest;
		this->FloorsNeeded = FloorsNeeded;
	}

	bool operator == (const FBuilding& OtherBuilding) const
	{
		return OtherBuilding.BuildingRequest == this->BuildingRequest;
	}
};
/**
 * Building subsystem used to queue buildings and floors for construction for RTS Agents
 */
UCLASS()
class MASSAITESTING_API URTSBuildingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	TArray<FBuilding> QueuedBuildings;

	TArray<FMassEntityHandle> RTSAgents;

public:
	UFUNCTION()
	void AddRTSAgent(const FMassEntityHandle& Entity);

	UFUNCTION(BlueprintCallable)
	void SelectClosestAgent(const FVector& Location);

	UFUNCTION(BlueprintCallable)
	void GetAgentLocation(FVector& OutLocation);

	UFUNCTION(BlueprintCallable)
	void GetAgentInformation(FRTSAgentFragment& OutAgentInfo);

	UPROPERTY()
	FMassEntityHandle RTSAgent;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FUpdateInventory OnEntityUpdateInventory;
	
	/**
	 * @brief Add a building to the subsystem for entities to build
	 * @param BuildingRequest Smart Object Handle of the building
	 * @param Floors The amount of floors to build
	 */
	UFUNCTION(BlueprintCallable)
	void AddBuilding(const FSmartObjectHandle& BuildingRequest, int Floors = 1);

	/**
	 * @brief Claim a 'random' building that needs construction of a floor
	 * @param Building OUT Smart Object handle used for completing the construction when resources are collected
	 */
	UFUNCTION()
	void ClaimFloor(FSmartObjectHandle& Building);
};
