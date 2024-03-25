// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeGotoRandomLocationTask.h"

#include "MassCommonFragments.h"
#include "MassSetSmartObjectMoveTargetTask.h"
#include "StateTreeLinker.h"
#include "MassAITesting/Mass/RTSItemTrait.h"

bool FMassStateTreeGotoRandomLocationTask::Link(FStateTreeLinker& Linker)
{
	//Linker.LinkInstanceDataProperty(RadiusHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeGotoRandomLocationTaskInstanceData, Radius));
	
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(MoveTargetHandle);
	return true;
}

EStateTreeRunStatus FMassStateTreeGotoRandomLocationTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMassMoveTargetFragment& MoveTargetFragment = Context.GetExternalData(MoveTargetHandle);
	const FTransform& Transform = Context.GetExternalData(TransformHandle).GetTransform();
	const float& Radius = Context.GetInstanceData<float>(*this);
	FVector RandomLocation = FVector(FMath::RandRange(-Radius, Radius), FMath::RandRange(-Radius, Radius), 0.f);
	RandomLocation += Transform.GetLocation();

	MoveTargetFragment.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTargetFragment.Center = RandomLocation;
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassStateTreeGotoRandomLocationTask::Tick(FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);

	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
	{
		MoveTarget.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
		return EStateTreeRunStatus::Succeeded;
	}
	
	return EStateTreeRunStatus::Running;
}
