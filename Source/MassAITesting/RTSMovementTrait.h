// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassProcessor.h"
#include "MassSignalSubsystem.h"
#include "SmartObjectSubsystem.h"
#include "RTSMovementTrait.generated.h"

class UMassEntitySubsystem;
class URTSMovementSubsystem;
class USmartObjectSubsystem;

UENUM()
enum EResourceType
{
	Tree,
	Rock
};

USTRUCT()
struct MASSAITESTING_API FRTSGatherResourceFragment : public FMassFragment
{
	GENERATED_BODY();

	// Type of resource to gather
	EResourceType Resource = Tree;

	// Amount of resource to gather
	int Amount = 0;
};

UCLASS()
class MASSAITESTING_API URTSGatherResourceProcessor : public UMassProcessor
{
	GENERATED_BODY()
public:
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;
};

USTRUCT()
struct MASSAITESTING_API FRTSAgentFragment : public FMassFragment
{
	GENERATED_BODY()
	
	FSmartObjectClaimHandle ClaimedObject;

	FVector SpawnLocation;

	TMap<EResourceType, int> Inventory;
	
	TMap<EResourceType, int> RequiredResources;
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
	TObjectPtr<UMassSignalSubsystem> SignalSubsystem;

	FMassEntityQuery EntityQuery;
};

USTRUCT()
struct MASSAITESTING_API FRTSAgent : public FMassTag
{
	GENERATED_BODY();
};
