// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassProcessor.h"
#include "SmartObjectSubsystem.h"
#include "RTSMovementTrait.generated.h"

class UMassEntitySubsystem;
class URTSMovementSubsystem;
class USmartObjectSubsystem;

USTRUCT()
struct MASSAITESTING_API FRTSMovementFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FSmartObjectClaimHandle ClaimedObject;

	FVector SpawnLocation;

	int WoodNeeded = 0;
	int RockNeeded = 0;
};

/**
 * 
 */
UCLASS()
class MASSAITESTING_API URTSMovementTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
};

UCLASS()
class MASSAITESTING_API URTSMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

	URTSMovementProcessor();
	
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	TObjectPtr<URTSMovementSubsystem> RTSMovementSubsystem;
	TObjectPtr<USmartObjectSubsystem> SmartObjectSubsystem;

	FMassEntityQuery EntityQuery;
};

USTRUCT()
struct MASSAITESTING_API FRTSAgent : public FMassTag
{
	GENERATED_BODY();
};
