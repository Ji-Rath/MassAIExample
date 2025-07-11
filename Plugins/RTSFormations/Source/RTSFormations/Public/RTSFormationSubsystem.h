// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FormationPresets.h"
#include "MassEntityHandle.h"
#include "MassSubsystemBase.h"
#include "RTSFormationSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FUnitInfo
{
	GENERATED_BODY()

	FUnitInfo() = default;
	
public:
	// Entities in the unit
	UPROPERTY()
	TSet<FMassEntityHandle> Entities;

	UPROPERTY()
	TMap<int, FVector> NewPositions;

	// The current unit position
	UPROPERTY()
	FVector UnitPosition = FVector::ZeroVector;

	// The direction to turn the unit when rotating
	UPROPERTY()
	float TurnDirection = 1.f;

	// The entity length of the 'front' of the unit
	UPROPERTY(BlueprintReadWrite)
	int FormationLength = 8;

	UPROPERTY(BlueprintReadWrite)
	float BufferDistance = 100.f;

	// The type of formation - WIP
	UPROPERTY(BlueprintReadWrite)
	EFormationType Formation = EFormationType::Rectangle;
	
	// Amount of rings for the circle formation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Rings = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHollow = false;

	UPROPERTY()
	FVector FarCorner;

	// Interpolated movement
	UPROPERTY()
	FVector InterpolatedDestination = FVector::ZeroVector;

	UPROPERTY()
	FRotator Rotation;
	
	UPROPERTY()
	FRotator InterpRotation;
	
	UPROPERTY()
	FRotator OldRotation;

	UPROPERTY()
	float InterpolationSpeed = 5.f;

	UPROPERTY()
	bool bBlendAngle = false;

	UPROPERTY()
	FVector ForwardDir;
};

class UMassAgentComponent;
struct FMassEntityHandle;

/**
 * Subsystem that handles the bulk of data shared among entities for the formation system. Enables simple unit creation and entity spawning
 */
UCLASS()
class RTSFORMATIONS_API URTSFormationSubsystem : public UMassTickableSubsystemBase
{
	GENERATED_BODY()
	
public:
	// Stores the num of units in the formation
	TArray<FMassEntityHandle> Units;

	// Destroy a specified entity
	UFUNCTION(BlueprintCallable)
	void DestroyEntity(UMassAgentComponent* Entity);

	// Set the position of a unit
	UFUNCTION()
	void UpdateUnitPosition(const FVector& NewPosition, const FMassEntityHandle& UnitHandle);

	UFUNCTION(BlueprintCallable)
	void SetUnitPositionByIndex(const FVector& NewPosition, int UnitIndex);
	
	void SetUnitPosition(const FVector& NewPosition, const FMassEntityHandle& UnitHandle);

	UFUNCTION()
	void MoveEntities(const FMassEntityHandle& UnitHandle);
	
	// Spawn entities for a unit
	UFUNCTION(BlueprintCallable)
	void SpawnEntitiesForUnitByIndex(int UnitIndex, const UMassEntityConfigAsset* EntityConfig, int Count);
	
	void SpawnEntitiesForUnit(const FMassEntityHandle& UnitHandle, const UMassEntityConfigAsset* EntityConfig, int Count);

	// Spawn a new unit
	UFUNCTION(BlueprintCallable)
	int SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count, const FVector& Position);

	UFUNCTION(BlueprintCallable)
	void SetFormationPresetByIndex(int EntityIndex, UFormationPresets* FormationAsset);
	
	void SetFormationPreset(const FMassEntityHandle& UnitHandle, UFormationPresets* FormationAsset);

	void CalculateNewPositions(const FMassEntityHandle& UnitHandle, TMap<int, FVector>& OutNewPositions);

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UPROPERTY()
	FTimerHandle MoveHandle;
	
	int CurrentIndex = 0;
};

template<>
struct TMassExternalSubsystemTraits<URTSFormationSubsystem>
{
	enum
	{
		GameThreadOnly = false
	};
};