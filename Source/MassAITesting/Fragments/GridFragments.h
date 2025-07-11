// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "GridFragments.generated.h"

USTRUCT()
struct FGridFragment : public FMassFragment
{
	GENERATED_BODY()

public:
	FGridFragment() = default;
	
	UPROPERTY()
	FVector LastQueriedPosition = FVector::ZeroVector;

	UPROPERTY()
	TArray<int32> NearbyNodes;
};


