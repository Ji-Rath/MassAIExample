// Fill out your copyright notice in the Description page of Project Settings.


#include "FollowPathTask.h"

#include "MassCommonFragments.h"
#include "MassNavigationFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"


bool FFollowPathTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	return FMassStateTreeTaskBase::Link(Linker);
}

void FFollowPathTask::MoveToNextPoint(FStateTreeExecutionContext& Context, EStateTreeRunStatus& StateTreeStatus) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const auto& TransformFragment = Context.GetExternalData(TransformHandle);
	const auto& PathPoints = InstanceData.InPath;
	if (PathPoints.IsEmpty())
	{
		StateTreeStatus = EStateTreeRunStatus::Succeeded;
		return;
	}

	// Create movement action
	MoveTarget.Center = PathPoints[InstanceData.PathIndex];
	MoveTarget.SlackRadius = 50.f;
	MoveTarget.Forward = (MoveTarget.Center - TransformFragment.GetTransform().GetLocation()).GetSafeNormal();
	MoveTarget.DistanceToGoal = FVector::Dist(MoveTarget.Center, TransformFragment.GetTransform().GetLocation());
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTarget.IntentAtGoal = EMassMovementAction::Stand;

	/*
	for(int i = InstanceData.PathIndex;i<InstanceData.InPath.Num()-1;i++)
	{
		DrawDebugLine(Context.GetWorld(), InstanceData.InPath[i], InstanceData.InPath[i+1], FColor::Red, false, 5.f, 0, 5);
	}
	*/

	InstanceData.PathIndex++;
	if (!PathPoints.IsValidIndex(InstanceData.PathIndex)) { StateTreeStatus = EStateTreeRunStatus::Succeeded; }
}

EStateTreeRunStatus FFollowPathTask::EnterState(FStateTreeExecutionContext& Context,
                                                const FStateTreeTransitionResult& Transition) const
{
	EStateTreeRunStatus Status = EStateTreeRunStatus::Running;
	MoveToNextPoint(Context, Status);

	return Status;
}

EStateTreeRunStatus FFollowPathTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	EStateTreeRunStatus Status = EStateTreeRunStatus::Running;

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const auto& TransformFragment = Context.GetExternalData(TransformHandle);

	if (FVector::Dist(TransformFragment.GetTransform().GetLocation(), MoveTarget.Center) < 100.f)
	{
		MoveToNextPoint(Context, Status);
	}

	return Status;
}

void FFollowPathTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.PathIndex = 0;
	
	FMassStateTreeTaskBase::ExitState(Context, Transition);
}

void FFollowPathTask::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.PathIndex = 0;
	FMassStateTreeTaskBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}
