// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeMoveToEntityHandle.h"

#include "MassCommandBuffer.h"
#include "MassEntitySubsystem.h"
#include "MassMovementFragments.h"
#include "MassSignalSubsystem.h"
#include "MassSmartObjectFragments.h"
#include "MassStateTreeExecutionContext.h"
#include "MassNavigationTypes.h"
#include "MassSetSmartObjectMoveTargetTask.h"
#include "StateTreeLinker.h"
#include "MassAITesting/RTSBuildingSubsystem.h"
#include "MassAITesting/Mass/RTSItemTrait.h"

bool FMassStateTreeMoveToEntityHandle::Link(FStateTreeLinker& Linker)
{
	//Linker.LinkInstanceDataProperty(EntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeMoveToEntityHandleInstanceData, ItemHandle));

	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SOUserHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MoveParametersHandle);
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMassStateTreeMoveToEntityHandle::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FMassMovementParameters& MoveParameters = Context.GetExternalData(MoveParametersHandle);
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassEntityHandle& ItemHandle = InstanceData.ItemHandle;
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);

	if (!EntitySubsystem.GetEntityManager().IsEntityValid(ItemHandle))
		return EStateTreeRunStatus::Failed;

	const FVector& Location = EntitySubsystem.GetEntityManager().GetFragmentDataChecked<FTransformFragment>(ItemHandle).GetTransform().GetLocation();
	
	MoveTarget.Center = Location;
	MoveTarget.SlackRadius = 100.f;
	MoveTarget.DesiredSpeed.Set(MoveParameters.DefaultDesiredSpeed);
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassStateTreeMoveToEntityHandle::Tick(FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	// When entity reaches target, mark as complete
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);

	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius)
	{
		MoveTarget.CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
		return EStateTreeRunStatus::Succeeded;
	}
	
	return EStateTreeRunStatus::Running;
}
