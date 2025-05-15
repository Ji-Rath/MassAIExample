// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSignalProcessorBase.h"
#include "Runtime/MassEntity/Public/MassProcessor.h"
#include "RandomMovementProcessors.generated.h"

/**
 * Processor that handles movement processing and distance calculations
 */
UCLASS()
class RANDOMMOVEMENT_API URandomMovementProcessors : public UMassProcessor
{
	GENERATED_BODY()

	URandomMovementProcessors();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

// Class that lets entities know of a new location to travel to
UCLASS()
class URandomMovementSignalProcessor : public UMassSignalProcessorBase
{
	GENERATED_BODY()

	URandomMovementSignalProcessor();

public:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;

	FMassEntityQuery EntityQuery;
};
