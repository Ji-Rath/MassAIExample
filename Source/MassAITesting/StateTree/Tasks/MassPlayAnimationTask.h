// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSmartObjectFragments.h"
#include "MassStateTreeMoveToEntityHandle.h"
#include "MassStateTreeTypes.h"
#include "MassAITesting/Mass/RTSAgentTrait.h"
#include "MassPlayAnimationTask.generated.h"

class UMassSignalSubsystem;
USTRUCT()
struct MASSAITESTING_API FMassPlayAnimationTaskInstanceData
{
	GENERATED_BODY()

	/** Delay before the task ends. Default (0 or any negative) will run indefinitely so it requires a transition in the state tree to stop it. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, Category = Parameter)
	int32 AnimationIndex = 0.f;

	/** Accumulated time used to stop task if duration is set */
	UPROPERTY()
	float Time = 0.f;
};

/**
 * 
 */
USTRUCT()
struct MASSAITESTING_API FMassPlayAnimationTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()
	
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassPlayAnimationTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;
	//virtual void ExitState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;
	//virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeHandle CompletedState) const {}
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
protected:
	//TStateTreeInstanceDataPropertyHandle<float> DurationHandle;
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FRTSAnimationFragment> AnimationHandle;

	TStateTreeInstanceDataPropertyHandle<float> TimeHandle;
	TStateTreeInstanceDataPropertyHandle<float> DurationHandle;
	TStateTreeInstanceDataPropertyHandle<int32> AnimationIndexHandle;
};
