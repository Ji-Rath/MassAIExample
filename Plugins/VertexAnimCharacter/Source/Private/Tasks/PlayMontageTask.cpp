// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayMontageTask.h"

#include "MassCommandBuffer.h"
#include "MassExecutionContext.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Mass/Animation/VertexAnimProcessor.h"

bool FPlayMontageTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	return Super::Link(Linker);
}

EStateTreeRunStatus FPlayMontageTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	auto& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	if (InstanceData.Montage.IsNull()) { return EStateTreeRunStatus::Failed; }

	FMassMontageFragment MontageData { InstanceData.Montage };
	MassContext.GetEntitySubsystemExecutionContext().Defer().PushCommand<FMassCommandAddFragmentInstances>(MassContext.GetEntity(), MontageData);
	InstanceData.MontageLength = InstanceData.Montage.LoadSynchronous()->GetPlayLength();

	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	SignalSubsystem.DelaySignalEntity(UE::Mass::Signals::ContextualAnimTaskFinished, MassContext.GetEntity(), InstanceData.MontageLength);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FPlayMontageTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	auto& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);

	InstanceData.TimeWaited += DeltaTime;

	if (InstanceData.TimeWaited > InstanceData.MontageLength)
	{
		return EStateTreeRunStatus::Running;
	}

	if (!MassContext.GetEntityManager().GetFragmentDataPtr<FMassMontageFragment>(MassContext.GetEntity()))
	{
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Running;
}

void FPlayMontageTask::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData<FInstanceDataType>(*this);
	auto& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	
	MassContext.GetEntityManager().Defer().RemoveFragment<FMassMontageFragment>(MassContext.GetEntity());
	InstanceData.MontageLength = 0.f;
	InstanceData.TimeWaited = 0.f;
}
