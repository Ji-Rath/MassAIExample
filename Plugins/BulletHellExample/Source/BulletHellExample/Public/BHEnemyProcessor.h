// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "BHEnemyProcessor.generated.h"

/**
 * Processor which manages the movement of enemies towards the player
 */
UCLASS()
class BULLETHELLEXAMPLE_API UBHEnemyProcessor : public UMassProcessor
{
	GENERATED_BODY()

	virtual void ConfigureQueries() override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;
};
