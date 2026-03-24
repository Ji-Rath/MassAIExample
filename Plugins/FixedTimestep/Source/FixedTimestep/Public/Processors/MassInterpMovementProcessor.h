// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassInterpMovementProcessor.generated.h"

/**
 * The translation layer between logic and visuals
 */
UCLASS()
class FIXEDTIMESTEP_API UMassInterpMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
public:
	UMassInterpMovementProcessor();
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;
};
