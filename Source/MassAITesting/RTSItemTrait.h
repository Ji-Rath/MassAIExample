// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTSAgentTrait.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "MassObserverProcessor.h"
#include "RTSItemTrait.generated.h"

struct FMassEntityTemplateBuildContext;
/**
 * 
 */
UCLASS()
class MASSAITESTING_API URTSItemTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const override;
};

USTRUCT()
struct MASSAITESTING_API FItemFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	TEnumAsByte<EResourceType> ItemType;

	UPROPERTY()
	FVector OldLocation;

	UPROPERTY()
	bool bClaimed = false;
};

USTRUCT()
struct MASSAITESTING_API FItemAddedToGrid : public FMassTag
{
	GENERATED_BODY()
};

UCLASS()
class MASSAITESTING_API UItemInitializerProcessor : public UMassObserverProcessor
{
	GENERATED_BODY()

	UItemInitializerProcessor();

public:
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;

	UPROPERTY()
	URTSBuildingSubsystem* BuildingSubsystem;
	
};

UCLASS()
class MASSAITESTING_API UItemProcessor : public UMassProcessor
{
	GENERATED_BODY()
	
	UItemProcessor();
public:
	virtual void Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries() override;
	virtual void Initialize(UObject& Owner) override;

	FMassEntityQuery EntityQuery;

	UPROPERTY()
	URTSBuildingSubsystem* BuildingSubsystem;

	UPROPERTY()
	UMassRepresentationSubsystem* RepresentationSubsystem;
};