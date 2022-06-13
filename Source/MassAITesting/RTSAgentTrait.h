// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassRepresentationSubsystem.h"
#include "MassSignalSubsystem.h"
#include "SmartObjectSubsystem.h"
#include "RTSAgentTrait.generated.h"

class UMassEntitySubsystem;
class URTSMovementSubsystem;
class USmartObjectSubsystem;

UENUM()
enum EResourceType
{
	Tree,
	Rock
};

/**
 * @brief Fragment given to entities to grant resources
 */
USTRUCT()
struct MASSAITESTING_API FRTSGatherResourceFragment : public FMassFragment
{
	GENERATED_BODY();

	// Type of resource to gather
	EResourceType Resource = Tree;

	// Amount of resource to gather
	int Amount = 0;
};

/**
 * @brief Processes FRTSGatherResourceFragment to grant resources to entity in FRTSAgent
 */
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

/**
 * @brief Base fragment for RTS Agents
 */
USTRUCT()
struct MASSAITESTING_API FRTSAgentFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "")
	TMap<TEnumAsByte<EResourceType>, int> Inventory;

	UPROPERTY(VisibleAnywhere, Category = "")
	TMap<TEnumAsByte<EResourceType>, int> RequiredResources;

	UPROPERTY()
	float SkinIndex = -1;
};

/**
 * @brief Base parameters for FRTSAgentTrait
 */
USTRUCT()
struct MASSAITESTING_API FRTSAgentParameters : public FMassSharedFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere);
	TMap<TEnumAsByte<EResourceType>, int> DefaultRequiredResources;
};

/**
 * @brief Declares this entity as an RTS Agent
 */
UCLASS()
class MASSAITESTING_API URTSAgentTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;

	UPROPERTY(EditAnywhere)
	FRTSAgentParameters AgentParameters;
};

/**
 * @brief Initializes RTS Agents with base required resources
 */
UCLASS()
class MASSAITESTING_API URTSAgentInitializer : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSAgentInitializer();
	
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;
	virtual void Register() override;

	TObjectPtr<URTSMovementSubsystem> RTSMovementSubsystem;
	TObjectPtr<USmartObjectSubsystem> SmartObjectSubsystem;

	FMassEntityQuery EntityQuery;
};

/**
 * Observer Processor to construct a smart object building floor
 */
UCLASS()
class MASSAITESTING_API URTSConstructBuilding : public UMassObserverProcessor
{
	GENERATED_BODY()

	URTSConstructBuilding();
	
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	TObjectPtr<URTSMovementSubsystem> RTSMovementSubsystem;
	TObjectPtr<USmartObjectSubsystem> SmartObjectSubsystem;

	float IncrementHeight = 100.f;

	FMassEntityQuery EntityQuery;
};

/**
 * @brief Setup ISMs, animation, and textures for RTS Agents
 */
UCLASS()
class URTSAnimationProcessor : public UMassProcessor
{
	GENERATED_BODY()

	virtual void Initialize(UObject& Owner) override;
	virtual void ConfigureQueries() override;
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;

	FMassEntityQuery EntityQuery;

	TObjectPtr<UMassRepresentationSubsystem> RepresentationSubsystem;
};

/**
 * @brief Fragment used to build floor in URTSConstructBuilding
 */
USTRUCT()
struct MASSAITESTING_API FRTSBuildingFragment : public FMassFragment
{
	GENERATED_BODY()

	FSmartObjectClaimHandle BuildingClaimHandle;
};

/**
 * @brief Basic tag to declare entity as RTS Agent
 */
USTRUCT()
struct MASSAITESTING_API FRTSAgent : public FMassTag
{
	GENERATED_BODY();
};

/**
 * @brief Tag used to request required resources from URTSAgentInitializer
 */
USTRUCT()
struct MASSAITESTING_API FRTSRequestResources : public FMassTag
{
	GENERATED_BODY()
};
