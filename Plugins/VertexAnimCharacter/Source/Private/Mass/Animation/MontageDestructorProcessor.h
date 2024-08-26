// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "MontageDestructorProcessor.generated.h"

/**
 * 
 */
UCLASS()
class VERTEXANIMCHARACTER_API UMontageDestructorProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()
public:
	UMontageDestructorProcessor();

	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
