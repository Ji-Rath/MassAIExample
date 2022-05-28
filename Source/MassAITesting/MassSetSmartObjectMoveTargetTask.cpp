// Fill out your copyright notice in the Description page of Project Settings.


#include "MassSetSmartObjectMoveTargetTask.h"

#include "MassSmartObjectFragments.h"
#include "MassStateTreeExecutionContext.h"

bool FMassSetSmartObjectMoveTargetTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SOUserHandle);
	
	return true;
}

EStateTreeRunStatus FMassSetSmartObjectMoveTargetTask::EnterState(FStateTreeExecutionContext& Context,
	const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	if (ChangeType != EStateTreeStateChangeType::Changed)
	{
		return EStateTreeRunStatus::Running;
	}
	
	// Update MoveTarget location
	//const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FMassSmartObjectUserFragment& SOUserFragment = Context.GetExternalData(SOUserHandle);

	if (!SOUserFragment.ClaimHandle.IsValid())
		return EStateTreeRunStatus::Failed;

	MoveTarget.Center = SOUserFragment.TargetLocation;
	MoveTarget.Forward = SOUserFragment.TargetDirection;
	MoveTarget.SlackRadius = 50.f;
	UE_LOG(LogTemp,Error,TEXT("ENTER STATE!"));

	return EStateTreeRunStatus::Running;
}

void FMassSetSmartObjectMoveTargetTask::ExitState(FStateTreeExecutionContext& Context,
	const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	if (ChangeType != EStateTreeStateChangeType::Changed)
	{
		return;
	}
	UE_LOG(LogTemp,Error,TEXT("EXITING STATE!"));
}

EStateTreeRunStatus FMassSetSmartObjectMoveTargetTask::Tick(FStateTreeExecutionContext& Context,
                                                            const float DeltaTime) const
{
	// When entity reaches target, mark as complete
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FTransform& Transform = Context.GetExternalData(TransformHandle).GetTransform();

	MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
	

	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
	{
		UE_LOG(LogTemp,Error,TEXT("REACHED TARGET!"));
		return EStateTreeRunStatus::Succeeded;
	}

	UE_LOG(LogTemp,Error,TEXT("DISTANCE: %d"),MoveTarget.DistanceToGoal);
	return EStateTreeRunStatus::Running;
}
