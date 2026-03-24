// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassFTCompositeProcessor.generated.h"

/**
 * 
 */
UCLASS()
class FIXEDTIMESTEP_API UMassFTCompositeProcessor : public UMassCompositeProcessor
{
	GENERATED_BODY()
	
public:
	UMassFTCompositeProcessor();
	
	virtual FGraphEventRef DispatchProcessorTasks(const TSharedPtr<FMassEntityManager>& EntityManager, FMassExecutionContext& ExecutionContext, const FGraphEventArray& Prerequisites = FGraphEventArray()) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	
	virtual void SubstepProcessors(float DeltaTime, const TFunction<void()>& SubstepFunction);
	
	FMassExecutionContext GetFixedTimestepContext(const FMassExecutionContext& InExecutionContext);
	
	UPROPERTY()
	float AccumulatedDeltaTime = 0.f;
};
