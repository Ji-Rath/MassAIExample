// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionContext.h"
#include "MassStateTreeTypes.h"
#include "MassEntityTypes.h"
#include "MassStateTreeGotoRandomLocationTask.generated.h"

struct FMassMoveTargetFragment;
struct FTransformFragment;

class URTSBuildingSubsystem;
USTRUCT()
struct FMassStateTreeGotoRandomLocationTaskInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Parameter)
	float Radius = 100.f;
};

/**
 * 
 */
USTRUCT()
struct MASSAITESTING_API FMassStateTreeGotoRandomLocationTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeGotoRandomLocationTaskInstanceData::StaticStruct(); }
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

	TStateTreeInstanceDataPropertyHandle<float> RadiusHandle;

	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<URTSBuildingSubsystem> BuildingSubsystemHandle;
};
