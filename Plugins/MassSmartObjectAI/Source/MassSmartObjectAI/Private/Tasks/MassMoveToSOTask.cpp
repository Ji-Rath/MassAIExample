// Fill out your copyright notice in the Description page of Project Settings.


#include "Tasks/MassMoveToSOTask.h"

#include "MassCommonFragments.h"
#include "MassStateTreeExecutionContext.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeLinker.h"

bool FMassMoveToSOTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	return Super::Link(Linker);
}

EStateTreeRunStatus FMassMoveToSOTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	// check if goal reached
	auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	auto& TransformFragment = Context.GetExternalData(TransformHandle);

	TRACE_CPUPROFILER_EVENT_SCOPE(ST_FindRandomLocation)
	MoveTarget.DistanceToGoal = FVector::Dist(MoveTarget.Center, TransformFragment.GetTransform().GetLocation());
	MoveTarget.Forward = (MoveTarget.Center - TransformFragment.GetTransform().GetLocation()).GetSafeNormal();
	
	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
	{
		MoveTarget.CreateNewAction(MoveTarget.IntentAtGoal, *Context.GetWorld());
		return EStateTreeRunStatus::Succeeded;
	}
	
	return EStateTreeRunStatus::Running;
}


EStateTreeRunStatus FMassMoveToSOTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	auto& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	auto& TransformFragment = Context.GetExternalData(TransformHandle);
	auto& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);

	// Create movement action
	const auto& ClaimHandle = InstanceData.ClaimHandle;
	auto Destination = SmartObjectSubsystem.GetSlotLocation(ClaimHandle);
	if (!Destination.IsSet()) { return EStateTreeRunStatus::Failed; }
	
	MoveTarget.Center = Destination.GetValue();
	MoveTarget.SlackRadius = 50.f;
	MoveTarget.Forward = (Destination.GetValue() - TransformFragment.GetTransform().GetLocation()).GetSafeNormal();
	MoveTarget.DistanceToGoal = FVector::Dist(MoveTarget.Center, TransformFragment.GetTransform().GetLocation());
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
	
	return EStateTreeRunStatus::Running;
}
