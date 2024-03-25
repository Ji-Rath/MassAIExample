// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSmartObjectFragments.h"
#include "MassStateTreeTypes.h"
#include "SmartObjectTypes.h"
#include "UObject/Object.h"
#include "MassSetSmartObjectMoveTargetTask.generated.h"

class UMassSignalSubsystem;
USTRUCT()
struct MASSAITESTING_API FMassSetSmartObjectMoveTargetInstanceData
{
	GENERATED_BODY()
};

/**
 * 
 */
USTRUCT()
struct MASSAITESTING_API FMassSetSmartObjectMoveTargetTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassSetSmartObjectMoveTargetInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	//virtual void ExitState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;
	//virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeHandle CompletedState) const {}
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
protected:
	//TStateTreeInstanceDataPropertyHandle<float> DurationHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<FMassSmartObjectUserFragment> SOUserHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FMassMovementParameters> MoveParametersHandle;
};
