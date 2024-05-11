// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassMovementFragments.h"
#include "MassProcessor.h"
#include "AdvancedRandomMovementTrait.generated.h"

class UMassEntitySubsystem;

USTRUCT()
struct MASSAITESTING_API FNPC : public FMassTag
{
	GENERATED_BODY();
};

/**
 * 
 */
UCLASS()
class MASSAITESTING_API UAdvancedRandomMovementTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};

UCLASS()
class MASSAITESTING_API UAdvancedRandomMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

	UAdvancedRandomMovementProcessor();
	
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;

	FMassEntityQuery EntityQuery;
};


