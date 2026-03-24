// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassFTProcessor.h"
#include "ApplyMovementFTProcessor.generated.h"

/**
 * 
 */
UCLASS()
class FIXEDTIMESTEP_API UApplyMovementFTProcessor : public UMassFTProcessor
{
	GENERATED_BODY()
	
public:
	UApplyMovementFTProcessor();
	
protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	FMassEntityQuery EntityQuery;
};
