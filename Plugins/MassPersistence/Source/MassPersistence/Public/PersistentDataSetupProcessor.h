// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "PersistentDataSetupProcessor.generated.h"

// Registers entities with the persistent data subsystem
UCLASS()
class UPersistentDataInitializerProcessor : public UMassObserverProcessor
{
public:
	UPersistentDataInitializerProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;

private:
	GENERATED_BODY()
};

// Unregisters entities with the persistent data subsystem
UCLASS()
class UPersistentDataDestructorProcessor : public UMassObserverProcessor
{
public:
	UPersistentDataDestructorProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;

private:
	GENERATED_BODY()
};
