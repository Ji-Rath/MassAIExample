// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Processors/MassFTProcessor.h"
#include "DesiredMovementProcessor.generated.h"

/**
 * 
 */
UCLASS()
class FIXEDTIMESTEP_API UDesiredMovementProcessor : public UMassFTProcessor
{
	GENERATED_BODY()
	
public:
	UDesiredMovementProcessor();
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;
};
