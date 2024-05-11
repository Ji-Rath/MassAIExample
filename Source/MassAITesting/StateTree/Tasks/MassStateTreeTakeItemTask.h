// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionContext.h"
#include "MassStateTreeTypes.h"
#include "MassEntityTypes.h"
#include "MassStateTreeTakeItemTask.generated.h"

class UMassEntitySubsystem;
class URTSBuildingSubsystem;
USTRUCT()
struct FMassStateTreeTakeItemTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = Input)
	FMassEntityHandle ItemHandle;
};

/**
 * 
 */
USTRUCT()
struct MASSAITESTING_API FMassStateTreeTakeItemTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassStateTreeTakeItemTaskInstanceData;
	
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeTakeItemTaskInstanceData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	//TStateTreeInstanceDataPropertyHandle<FMassEntityHandle> EntityHandle;

	TStateTreeExternalDataHandle<UMassEntitySubsystem> EntitySubsystemHandle;
	TStateTreeExternalDataHandle<URTSBuildingSubsystem> BuildingSubsystemHandle;
};
