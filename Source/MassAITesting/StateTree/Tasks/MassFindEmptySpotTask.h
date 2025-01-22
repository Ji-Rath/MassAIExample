// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "MassFindEmptySpotTask.generated.h"

struct FTransformFragment;
class UGridManagerSubsystem;

USTRUCT()
struct MASSAITESTING_API FMassFindEmptySpotTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Output)
	FVector EmptySpot;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Find Empty Spot Task"))
struct MASSAITESTING_API FMassFindEmptySpotTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassFindEmptySpotTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassFindEmptySpotTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	TStateTreeExternalDataHandle<UGridManagerSubsystem> GridManagerHandle;
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
};