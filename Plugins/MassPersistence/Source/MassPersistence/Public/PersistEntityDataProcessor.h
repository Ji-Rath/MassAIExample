// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"

#include "PersistEntityDataProcessor.generated.h"

/**
 * The persistent data processor is meant to receive signals for the entities that we want to save
 * The processor will then read and save data so that it is persisted.
 * The idea behind this processor is to make sure that any data persisted is explicit as saving/loading may be costly for many entities
 */
UCLASS()
class MASSPERSISTENCE_API UPersistEntityDataProcessor : public UMassSignalProcessorBase
{
protected:
	virtual void ConfigureQueries() override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
		FMassSignalNameLookup& EntitySignals) override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;

private:
	GENERATED_BODY()
};

// Processor which manages entities which have just been loaded
// This is a good spot to do any cleanup/setup for entities after they are spawned (from a savegame)
UCLASS()
class MASSPERSISTENCE_API UPersistentDataPostLoadProcessor : public UMassSignalProcessorBase
{
protected:
	virtual void ConfigureQueries() override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
		FMassSignalNameLookup& EntitySignals) override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;

private:
	GENERATED_BODY()
};
