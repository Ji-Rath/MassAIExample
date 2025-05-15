// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "BHEnemyProcessor.generated.h"

/**
 * Processor which manages the movement of enemies towards the player
 */
UCLASS()
class BULLETHELLEXAMPLE_API UBHEnemyProcessor : public UMassProcessor
{
	GENERATED_BODY()

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	FMassEntityQuery UpdateHashGridQuery;
};

UCLASS()
class UBHEnemyInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UBHEnemyInitializer();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};

UCLASS()
class UBHEnemyDestructor : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UBHEnemyDestructor();
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
