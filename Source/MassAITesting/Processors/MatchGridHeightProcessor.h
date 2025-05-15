// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MatchGridHeightProcessor.generated.h"

/**
 * Processor which ensures that entities are maintaining their position snapped to the landscape grid
 */
UCLASS()
class MASSAITESTING_API UMatchGridHeightProcessor : public UMassProcessor
{
	GENERATED_BODY()

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	FMassEntityQuery EntityQuery;
};
