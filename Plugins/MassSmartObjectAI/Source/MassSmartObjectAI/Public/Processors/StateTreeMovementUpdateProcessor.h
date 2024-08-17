// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "StateTreeMovementUpdateProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSSMARTOBJECTAI_API UStateTreeMovementUpdateProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
	UStateTreeMovementUpdateProcessor();
protected:
	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
