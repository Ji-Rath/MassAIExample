// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HierarchicalHashGrid2D.h"
#include "MassEntityTypes.h"
#include "MassSubsystemBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "HashGridSubsystem.generated.h"


typedef THierarchicalHashGrid2D<2, 4, FMassEntityHandle> FHashGridExample;	// 2 levels of hierarchy, 4 ratio between levels

/**
 * 
 */
UCLASS()
class SPATIALHASHGRIDEXAMPLE_API UHashGridSubsystem : public UMassSubsystemBase
{
	GENERATED_BODY()

public:
	FHashGridExample HashGridData;

	UFUNCTION(BlueprintCallable)
	void SelectEntitiesInArea(const FVector& SelectedLocation, float Radius);
};

template<>
struct TMassExternalSubsystemTraits<UHashGridSubsystem> final
{
	enum
	{
		GameThreadOnly = false
	};
};
