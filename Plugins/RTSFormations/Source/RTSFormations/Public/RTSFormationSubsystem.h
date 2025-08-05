// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FormationPresets.h"
#include "MassEntityHandle.h"
#include "MassSubsystemBase.h"
#include "Unit/UnitFragments.h"
#include "RTSFormationSubsystem.generated.h"

struct FMassEntityQuery;
struct FMassExecutionContext;
struct FMassEntityConfig;
class UMassEntityConfigAsset;
class UMassAgentComponent;
struct FMassEntityHandle;

namespace RTS::Stats
{
	inline double UpdateEntityIndexTimeSec = 0.0;
	inline double UpdateUnitPositionTimeSec = 0.0;
}

/**
 * Subsystem that handles the bulk of data shared among entities for the formation system. Enables simple unit creation and entity spawning
 */
UCLASS()
class RTSFORMATIONS_API URTSFormationSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()
	
public:
	TArray<FUnitHandle> GetUnits() const;

	UFUNCTION(BlueprintCallable)
	FUnitHandle GetFirstUnit() const;
	
	// Destroy a specified entity
	UFUNCTION(BlueprintCallable)
	void DestroyEntity(UMassAgentComponent* Entity);

	// Set the position of a unit
	void UpdateUnitPosition(const FUnitHandle& UnitHandle);

	UFUNCTION(BlueprintCallable)
	void SetUnitPosition(const FVector& NewPosition, const FUnitHandle& UnitHandle);
	
	// Spawn entities for a unit
	UFUNCTION(BlueprintCallable)
	void SpawnEntitiesForUnit(const FUnitHandle& UnitHandle, const UMassEntityConfigAsset* EntityConfig, int Count);

	void SpawnEntities(const FUnitHandle& UnitHandle, const FMassEntityConfig& EntityConfig, int Count);

	// Spawn a new unit
	UFUNCTION(BlueprintCallable)
	FUnitHandle SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count, const FVector& Position);

	FUnitHandle SpawnUnit(const FMassEntityConfig& EntityConfig, int Count, const FVector& Position);
	
	void SetFormationPreset(const FUnitHandle& UnitHandle, UFormationPresets* FormationAsset);

	static void CalculateNewPositions(FUnitFragment& UnitFragment, int Count, TArray<FVector3f>& OutNewPositions);

	static void CreateQueryForUnit(const FUnitHandle& UnitHandle, FMassEntityQuery& EntityQuery);
};

template<>
struct TMassExternalSubsystemTraits<URTSFormationSubsystem> final
{
	enum
	{
		GameThreadOnly = false,
		ThreadSafeWrite = false
	};
};