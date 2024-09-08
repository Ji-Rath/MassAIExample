// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"

#include "HashGridExampleProcessors.generated.h"



/**
 * When an entity is given the specified fragment, we do initialization so that the entity and position are added to the hash grid
 */
UCLASS()
class SPATIALHASHGRIDEXAMPLE_API UHashGridInitializeProcessor : public UMassObserverProcessor
{
 GENERATED_BODY()
 UHashGridInitializeProcessor();
 virtual void ConfigureQueries() override;
 virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
 FMassEntityQuery EntityQuery;
};

/**
 * When the fragment is removed from the entity, we assume that we no longer want the entity to be tracked by the hash grid. Do appropriate cleanup here
 */
UCLASS()
class SPATIALHASHGRIDEXAMPLE_API UHashGridDestroyProcessor : public UMassObserverProcessor
{
 GENERATED_BODY()
 UHashGridDestroyProcessor();
 virtual void ConfigureQueries() override;
 virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
 FMassEntityQuery EntityQuery;
};

/**
 * This processor will mainly update the entities that are within the hash grid and update their position. This will allow for accurate queries when the entities move
 */
UCLASS()
class SPATIALHASHGRIDEXAMPLE_API UHashGridProcessor : public UMassProcessor
{
 GENERATED_BODY()
 virtual void ConfigureQueries() override;
 virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
 FMassEntityQuery EntityQuery;
};


/**
 * Performs simple logic when an entity receives that they should be affected
 * This is mainly to show the query of the hash grid and affecting entities
 */
UCLASS()
class UHashGridQueryProcessor : public UMassSignalProcessorBase
{
 GENERATED_BODY()

 virtual void Initialize(UObject& Owner) override;
 virtual void ConfigureQueries() override;
 virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;
};
