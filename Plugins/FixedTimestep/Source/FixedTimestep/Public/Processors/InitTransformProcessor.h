// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MassSignalProcessorBase.h"
#include "InitTransformProcessor.generated.h"

/**
 * 
 */
UCLASS()
class FIXEDTIMESTEP_API UInitTransformProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
	
public:
	UInitTransformProcessor();
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;
};
