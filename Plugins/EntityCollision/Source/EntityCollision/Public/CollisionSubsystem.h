// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HierarchicalHashGrid2D.h"
#include "MassSubsystemBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "CollisionSubsystem.generated.h"

USTRUCT()
struct FCollisionProxyHandle
{
	GENERATED_BODY()
	
public:
	FCollisionProxyHandle() = default;
	
	FCollisionProxyHandle(const FMassEntityHandle& InHandle, const FVector& InLocation, const FVector& InVelocity)
		: EntityLocation(InLocation)
		, EntityVelocity(InVelocity)
		, EntityHandle(InHandle)
	{
	}
	
	FCollisionProxyHandle(const FMassEntityHandle& InHandle)
		: EntityHandle(InHandle)
	{
		EntityLocation = FVector::ZeroVector;
		EntityVelocity = FVector::ZeroVector;
	}
	
	// We only need to verify the entity handle to check equality
	bool operator==(const FCollisionProxyHandle& OtherHandle) const
	{
		return EntityHandle == OtherHandle.EntityHandle;
	}
	
	bool operator!=(const FCollisionProxyHandle& OtherHandle) const
	{
		return EntityHandle != OtherHandle.EntityHandle;
	}
	
	FVector EntityLocation;
	FVector EntityVelocity;
	FMassEntityHandle EntityHandle;
};


class UMassEntityConfigAsset;
typedef THierarchicalHashGrid2D<1, 1, FCollisionProxyHandle> FHashGridExample;	// 2 levels of hierarchy, 4 ratio between levels

/**
 * 
 */
UCLASS()
class ENTITYCOLLISION_API UCollisionSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()

public:
	FHashGridExample HashGridData = FHashGridExample(100);

	UFUNCTION(BlueprintCallable)
	void SpawnEntities(const FVector& Location, int Count, UMassEntityConfigAsset* EntityConfig);
};

template<>
struct TMassExternalSubsystemTraits<UCollisionSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
