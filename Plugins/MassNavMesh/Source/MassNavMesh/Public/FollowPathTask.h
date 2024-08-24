// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "FollowPathTask.generated.h"

struct FTransformFragment;
struct FMassMoveTargetFragment;

USTRUCT()
struct FFollowPathTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<FVector> InPath;

	int PathIndex = 0;
};

USTRUCT(meta = (DisplayName = "Follow Nav Path Task"))
struct MASSNAVMESH_API FFollowPathTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FFollowPathTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	void MoveToNextPoint(FStateTreeExecutionContext& Context, EStateTreeRunStatus& StateTreeStatus) const;
	virtual const UStruct* GetInstanceDataType() const override { return FFollowPathTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const override;
protected:
	TStateTreeExternalDataHandle<FMassMoveTargetFragment> MoveTargetHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
};
