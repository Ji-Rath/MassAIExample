// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MoveTargetProcessor.generated.h"

class UMassSignalSubsystem;
/**
 * 
 */
UCLASS()
class MASSAITESTING_API UMoveTargetProcessor : public UMassProcessor
{
	GENERATED_BODY()

	UMoveTargetProcessor();

protected:
	virtual void ConfigureQueries() override;

	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	virtual void Initialize(UObject& Owner) override;

private:
	FMassEntityQuery EntityQuery;
	
	TObjectPtr<UMassSignalSubsystem> SignalSubsystem;
};
