// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HierarchicalHashGrid2D.h"
#include "MassEntityHandle.h"
#include "MassEntityTypes.h"
#include "MassSubsystemBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "CollisionSubsystem.generated.h"


class UMassEntityConfigAsset;
typedef THierarchicalHashGrid2D<2, 4, FMassEntityHandle> FHashGridExample;	// 2 levels of hierarchy, 4 ratio between levels

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
