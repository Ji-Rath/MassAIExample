// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "CalculatePathTask.generated.h"

class UMassSignalSubsystem;
struct FTransformFragment;
struct FMassMoveTargetFragment;

USTRUCT()
struct FCalculatePathTaskInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Input")
	FVector DesiredLocation;

	UPROPERTY(EditAnywhere, Category="Output")
	TArray<FVector> OutputPath;

	UPROPERTY(EditAnywhere, Category="Output")
	bool bFoundPath = false;

	int QueryID = -1;
};

USTRUCT(meta = (DisplayName = "Calculate Nav Path Task"))
struct MASSNAVMESH_API FCalculatePathTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FCalculatePathTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FCalculatePathTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus, const FStateTreeActiveStates& CompletedActiveStates) const override;
protected:
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> SignalSubsystemHandle;
};
