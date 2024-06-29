// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeTypes.h"
#include "StateTreeExecutionTypes.h"
#include "MassWaitTask.generated.h"

/**
 * Task to assign a LookAt target for mass processing
 */
USTRUCT()
struct MASSAITESTING_API FMassWaitTaskInstanceData
{
	GENERATED_BODY()
 
	/** Delay before the task ends. Default (0 or any negative) will run indefinitely so it requires a transition in the state tree to stop it. */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float Duration = 0.f;

	float TimePassed = 0.f;
};

/**
 * 
 */
USTRUCT(meta = (DisplayName = "Mass Wait Task"))
struct MASSAITESTING_API FMassWaitTask : public FMassStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassWaitTaskInstanceData;

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassWaitTaskInstanceData::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

protected:
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};
