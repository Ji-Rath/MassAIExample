// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "BulletProcessor.generated.h"

/**
 * Processor which manages bullets in the world
 */
UCLASS()
class BULLETHELLEXAMPLE_API UBulletInitializerProcessor : public UMassSignalProcessorBase
{
	GENERATED_BODY()

	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;

	FMassEntityQuery EntityQuery;
};

UCLASS()
class UBulletDestroyerProcessor : public UMassSignalProcessorBase
{
	GENERATED_BODY()

public:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context, FMassSignalNameLookup& EntitySignals) override;

	FMassEntityQuery EntityQuery;
};

UCLASS()
class UBulletCollisionProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
