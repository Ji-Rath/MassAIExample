// Fill out your copyright notice in the Description page of Project Settings.


#include "MassMoveTask.h"

#include "MassStateTreeExecutionContext.h"

bool FMassMoveTask::Link(FStateTreeLinker& Linker)
{
	//Linker.LinkExternalData(MassSignalSubsystemHandle);
	//Linker.LinkExternalData(PathHandle);

	//Linker.LinkInstanceDataProperty(DurationHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassMoveTaskInstanceData, Duration));
	//Linker.LinkInstanceDataProperty(TargetEntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassLookAtTaskInstanceData, TargetEntity));
	//Linker.LinkInstanceDataProperty(TimeHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassLookAtTaskInstanceData, Time));
	
	return true;
}

EStateTreeRunStatus FMassMoveTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return EStateTreeRunStatus::Succeeded;
}


EStateTreeRunStatus FMassMoveTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const float Duration = Context.GetInstanceData()
	return EStateTreeRunStatus::Failed;
}
