// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"

#include "CollisionProcessors.generated.h"


/**
 * When an entity is given the specified fragment, we do initialization so that the entity and position are added to the hash grid
 */
UCLASS()
class ENTITYCOLLISION_API UCollisionInitializerProcessor : public UMassObserverProcessor
{
 GENERATED_BODY()
 UCollisionInitializerProcessor();
 virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
 virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
 FMassEntityQuery EntityQuery;
};

/**
 * When the fragment is removed from the entity, we assume that we no longer want the entity to be tracked by the hash grid. Do appropriate cleanup here
 */
UCLASS()
class ENTITYCOLLISION_API UCollisionDestroyProcessor : public UMassObserverProcessor
{
 GENERATED_BODY()
 UCollisionDestroyProcessor();
 virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
 virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
 FMassEntityQuery EntityQuery;
};

/**
 * This processor will mainly update the entities that are within the hash grid and update their position. This will allow for accurate queries when the entities move
 */
UCLASS()
class ENTITYCOLLISION_API UCollisionProcessor : public UMassProcessor
{
 GENERATED_BODY()
 UCollisionProcessor();
 virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
 virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
 FMassEntityQuery EntityQuery;
 FMassEntityQuery CollisionQuery;

 FVector ResolveCollisions(const TArray<FMassEntityHandle>& Entities, FMassEntityManager& EntityManager, float Radius,
                           FTransform
                           & EntityTransform);
};
