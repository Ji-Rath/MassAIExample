// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "UObject/Object.h"
#include "SimpleRandomMovementProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSAITESTING_API USimpleRandomMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

	USimpleRandomMovementProcessor();

protected:
	virtual void ConfigureQueries() override;

	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
