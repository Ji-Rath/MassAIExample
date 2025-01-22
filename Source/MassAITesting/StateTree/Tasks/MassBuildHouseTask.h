// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "MassBuildHouseTask.generated.h"

struct FResourceUserFragment;

USTRUCT()
struct MASSAITESTING_API FMassBuildHouseTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Parameter)
	TSoftClassPtr<AActor> House;

	UPROPERTY(EditAnywhere, Category = Input)
	FVector Location;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Build House Task"))
struct MASSAITESTING_API FMassBuildHouseTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassBuildHouseTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassBuildHouseTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	TStateTreeExternalDataHandle<FResourceUserFragment> ResourceUserHandle;
};
