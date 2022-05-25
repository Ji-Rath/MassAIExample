// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassProcessor.h"
#include "NavigationPath.h"
#include "NavMeshMovementTrait.generated.h"

class UMassEntitySubsystem;

USTRUCT()
struct MASSAITESTING_API FNavMeshPathFragment : public FMassFragment
{
	GENERATED_BODY()
	
	UPROPERTY()
	int8 PathIndex = 0;

	UPROPERTY(EditAnywhere)
	FVector TargetLocation;
	
	FPathFindingResult PathResult;
	
	void SetTargetLocation(const FVector& Target)
	{
		TargetLocation = Target;
		PathIndex = 0;
	}
};

/**
 * 
 */
UCLASS()
class MASSAITESTING_API UNavMeshMovementTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
};

UCLASS()
class MASSAITESTING_API UNavMeshMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

	UNavMeshMovementProcessor();
	
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;

	FMassEntityQuery EntityQuery;
};

USTRUCT()
struct MASSAITESTING_API FNavAgent : public FMassTag
{
	GENERATED_BODY();
};
