// Fill out your copyright notice in the Description page of Project Settings.


#include "MassWaitTask.h"

#include "MassSignalSubsystem.h"
#include "MassStateTreeExecutionContext.h"
#include "MassStateTreeTypes.h"
#include "StateTreeLinker.h"

bool FMassWaitTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMassWaitTask::EnterState(FStateTreeExecutionContext& Context,
                                              const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	auto& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::StateTreeActivate, MassContext.GetEntity(), InstanceData.Duration);
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassWaitTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.TimePassed += DeltaTime;

	if (InstanceData.TimePassed >= InstanceData.Duration)
	{
		return EStateTreeRunStatus::Succeeded;
	}
	else
	{
		const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
		auto& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
		MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::StateTreeActivate, MassContext.GetEntity(), InstanceData.Duration - InstanceData.TimePassed);
	}
	
	return EStateTreeRunStatus::Running;
}

void FMassWaitTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMassStateTreeTaskBase::ExitState(Context, Transition);

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.TimePassed = 0.f;
}
