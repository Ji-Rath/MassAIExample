// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "TickSTProcessor.generated.h"

/**
 * Processor whos function is to tick the state tree at certain intervals.
 * This may fluctuate due to distance, LOD, or other factors.
 */
UCLASS()
class MASSSMARTOBJECTAI_API UTickSTProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
