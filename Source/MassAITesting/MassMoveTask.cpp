// Fill out your copyright notice in the Description page of Project Settings.


#include "MassMoveTask.h"

#include "MassStateTreeExecutionContext.h"
#include "NavMeshMovementTrait.h"

bool FMassMoveTask::Link(FStateTreeLinker& Linker)
{
	//Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(PathHandle);

	Linker.LinkInstanceDataProperty(DurationHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassMoveTaskInstanceData, Duration));
	//Linker.LinkInstanceDataProperty(TargetEntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassLookAtTaskInstanceData, TargetEntity));
	//Linker.LinkInstanceDataProperty(TimeHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassLookAtTaskInstanceData, Time));
	
	return true;
}

EStateTreeRunStatus FMassMoveTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	UE_LOG(LogTemp, Error, TEXT("TASK RUNNING!"));
	
	return EStateTreeRunStatus::Running;
}


EStateTreeRunStatus FMassMoveTask::EnterState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	UE_LOG(LogTemp, Error, TEXT("ENTER STATE!"));
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	FNavMeshPathFragment& PathFragment = MassContext.GetExternalData(PathHandle);
	const float Duration = Context.GetInstanceData(DurationHandle);
	return EStateTreeRunStatus::Running;
}
