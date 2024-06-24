// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "MW_FindRandomLocationTask.generated.h"

class UMassSignalSubsystem;
struct FTransformFragment;

USTRUCT()
struct MASSWANDERAI_API FMW_FindRandomLocationTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Input)
	float Range = 1000.f;

	UPROPERTY(EditAnywhere, Category=Output)
	FVector OutLocation;

	FMW_FindRandomLocationTaskInstanceData() = default;
};

/**
 * Find a random location to wander to
 */
USTRUCT(meta = (DisplayName = "MW Find Random Location"))
struct MASSWANDERAI_API FMW_FindRandomLocationTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMW_FindRandomLocationTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMW_FindRandomLocationTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
protected:
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
};
