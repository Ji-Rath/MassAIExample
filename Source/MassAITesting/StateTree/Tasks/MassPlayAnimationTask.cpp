// Fill out your copyright notice in the Description page of Project Settings.


#include "MassPlayAnimationTask.h"

#include "MassSignalSubsystem.h"
#include "MassStateTreeExecutionContext.h"

bool FMassPlayAnimationTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(AnimationHandle);
	Linker.LinkInstanceDataProperty(DurationHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassPlayAnimationTaskInstanceData, Duration));
	Linker.LinkInstanceDataProperty(TimeHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassPlayAnimationTaskInstanceData, Time));
	Linker.LinkInstanceDataProperty(AnimationIndexHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassPlayAnimationTaskInstanceData, AnimationIndex));
	
	return true;
}

EStateTreeRunStatus FMassPlayAnimationTask::EnterState(FStateTreeExecutionContext& Context,
	const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	// Update MoveTarget location
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	FRTSAnimationFragment& AnimationFragment = Context.GetExternalData(AnimationHandle);

	float& Time = Context.GetInstanceData(TimeHandle);
	Time = 0;

	AnimationFragment.bCustomAnimation = true;
	AnimationFragment.AnimPosition = 0;
	AnimationFragment.AnimationStateIndex = Context.GetInstanceData(AnimationIndexHandle);
	MoveTarget.CreateNewAction(EMassMovementAction::Animate, *Context.GetWorld());

	const float Duration = Context.GetInstanceData(DurationHandle);
	if (Duration > 0.0f)
	{
		UMassSignalSubsystem& MassSignalSubsystem = MassContext.GetExternalData(MassSignalSubsystemHandle);
		MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::LookAtFinished, MassContext.GetEntity(), Duration);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassPlayAnimationTask::Tick(FStateTreeExecutionContext& Context,
                                                            const float DeltaTime) const
{
	// When entity reaches target, mark as complete
	const FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	FRTSAnimationFragment& AnimationFragment = Context.GetExternalData(AnimationHandle);

	float& Time = Context.GetInstanceData(TimeHandle);
	const float Duration = Context.GetInstanceData(DurationHandle);
	
	Time += DeltaTime;

	if (Time >= Duration)
	{
		AnimationFragment.bCustomAnimation = false;
	}
	else
	{
		const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
		UMassSignalSubsystem& MassSignalSubsystem = MassContext.GetExternalData(MassSignalSubsystemHandle);
		MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::LookAtFinished, MassContext.GetEntity(), Duration);
	}
		
	
	return Duration <= 0.0f ? EStateTreeRunStatus::Running : (Time < Duration ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded);

}
