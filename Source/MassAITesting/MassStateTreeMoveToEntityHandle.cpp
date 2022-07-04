// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeMoveToEntityHandle.h"
#include "MassMovementFragments.h"
#include "MassSignalSubsystem.h"
#include "MassSmartObjectFragments.h"
#include "MassStateTreeExecutionContext.h"
#include "MassNavigationTypes.h"
#include "MassSetSmartObjectMoveTargetTask.h"
#include "RTSBuildingSubsystem.h"

bool FMassStateTreeMoveToEntityHandle::Link(FStateTreeLinker& Linker)
{
	Linker.LinkInstanceDataProperty(EntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeMoveToEntityHandleInstanceData, ItemHandle));

	Linker.LinkExternalData(MoveTargetHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SOUserHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(MoveParametersHandle);
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMassStateTreeMoveToEntityHandle::EnterState(FStateTreeExecutionContext& Context,
                                                                 const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FMassSmartObjectUserFragment& SOUserFragment = Context.GetExternalData(SOUserHandle);
	const FMassMovementParameters& MoveParameters = Context.GetExternalData(MoveParametersHandle);
	const FMassEntityHandle& ItemHandle = Context.GetInstanceData(EntityHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);

	if (!EntitySubsystem.IsEntityValid(ItemHandle))
		return EStateTreeRunStatus::Failed;

	const FVector& Location = EntitySubsystem.GetFragmentDataChecked<FTransformFragment>(ItemHandle).GetTransform().GetLocation();
	
	MoveTarget.Center = Location;
	MoveTarget.SlackRadius = 25.f;
	MoveTarget.DesiredSpeed.Set(MoveParameters.DefaultDesiredSpeed);
	MoveTarget.CreateNewAction(EMassMovementAction::Move, *Context.GetWorld());
	MoveTarget.IntentAtGoal = EMassMovementAction::Stand;
	
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FMassStateTreeMoveToEntityHandle::Tick(FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	const FMassEntityHandle& ItemHandle = Context.GetInstanceData(EntityHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	UMassSignalSubsystem& MassSignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	URTSBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::FollowPointPathDone, MassContext.GetEntity(), 1);
	
	// When entity reaches target, mark as complete
	FMassMoveTargetFragment& MoveTarget = Context.GetExternalData(MoveTargetHandle);
	const FTransform& Transform = Context.GetExternalData(TransformHandle).GetTransform();

	MoveTarget.DistanceToGoal = (MoveTarget.Center - Transform.GetLocation()).Length();
	MoveTarget.Forward = (MoveTarget.Center - Transform.GetLocation()).GetSafeNormal();

	if (MoveTarget.DistanceToGoal <= MoveTarget.SlackRadius+100.f)
	{
		EntitySubsystem.Defer().DestroyEntity(ItemHandle);
		BuildingSubsystem.ItemHashGrid.RemovePoint(ItemHandle, MoveTarget.Center);
		return EStateTreeRunStatus::Succeeded;
	}
	
	return EStateTreeRunStatus::Running;
}
