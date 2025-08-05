// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSignalProcessorBase.h"
#include "UnitFragments.h"
#include "UpdateUnitPositionProcessor.generated.h"

// General ticking for unit shared fragments
UCLASS()
class UUnitProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery = FMassEntityQuery(*this);
};
