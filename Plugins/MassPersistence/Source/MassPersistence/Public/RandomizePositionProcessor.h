// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassSignalProcessorBase.h"
#include "RandomizePositionProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSPERSISTENCE_API URandomizePositionProcessor : public UMassSignalProcessorBase
{
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void SignalEntities(FMassEntityManager& EntityManager, FMassExecutionContext& Context,
		FMassSignalNameLookup& EntitySignals) override;
	virtual void InitializeInternal(UObject& Owner, const TSharedRef<FMassEntityManager>& EntityManager) override;

	FMassEntityQuery EntityQuery;

private:
	GENERATED_BODY()
};
