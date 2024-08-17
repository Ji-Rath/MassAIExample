// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/Tasks/MW_FindRandomLocationTask.h"

#include "MassCommonFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FMW_FindRandomLocationTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TransformHandle);
	
	return FMassStateTreeTaskBase::Link(Linker);
}

EStateTreeRunStatus FMW_FindRandomLocationTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (Transition.ChangeType != EStateTreeStateChangeType::Changed) { return EStateTreeRunStatus::Running; }
	TRACE_CPUPROFILER_EVENT_SCOPE(ST_FindRandomLocation)
	
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const float Range = InstanceData.Range;
	FTransformFragment& Transform = Context.GetExternalData(TransformHandle);
	const auto NewOffset = FVector(FMath::RandRange(Range/2*-1, Range/2), FMath::RandRange(Range/2*-1, Range/2), 0.f);;

	InstanceData.OutLocation = Transform.GetTransform().GetLocation()+NewOffset;
	
	return EStateTreeRunStatus::Running;
}
