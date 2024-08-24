// Fill out your copyright notice in the Description page of Project Settings.

#include "CalculatePathTask.h"

#include "MassCommonFragments.h"
#include "MassSignalSubsystem.h"
#include "MassStateTreeExecutionContext.h"
#include "NavigationSystem.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "AI/NavigationSystemBase.h"

bool FCalculatePathTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(SignalSubsystemHandle);
	return FMassStateTreeTaskBase::Link(Linker);
}

EStateTreeRunStatus FCalculatePathTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	auto& TransformFragment = Context.GetExternalData(TransformHandle);

	if (InstanceData.QueryID == -1) { return EStateTreeRunStatus::Running; }
	
	auto NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Context.GetWorld());
	FNavAgentProperties NavProperties{ 25, 60};
	auto NavData = NavSystem->GetNavDataForProps(NavProperties);
	if (!NavData) { return EStateTreeRunStatus::Failed; }

	InstanceData.OutputPath.Empty();
	auto& SignalSubsystem = Context.GetExternalData(SignalSubsystemHandle);
	const auto& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	auto Entity = MassContext.GetEntity();
	
	FPathFindingQuery PathQuery{ Context.GetWorld(), *NavData, TransformFragment.GetTransform().GetLocation(), InstanceData.DesiredLocation };
	InstanceData.QueryID = NavSystem->FindPathAsync(NavProperties, PathQuery, FNavPathQueryDelegate::CreateLambda([&InstanceData, &SignalSubsystem, Entity, &MassContext](uint32 QueryID, ENavigationQueryResult::Type Result, FNavPathSharedPtr NavPath)
	{
		switch (Result)
		{
		case ENavigationQueryResult::Success:
			for (const auto& PathPoint : NavPath->GetPathPoints())
			{
				InstanceData.OutputPath.Emplace(PathPoint.Location);
			}
			InstanceData.bFoundPath = true;
			break;
		default: break;
		}
	}));
	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FCalculatePathTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return FMassStateTreeTaskBase::Tick(Context, DeltaTime);
}

void FCalculatePathTask::StateCompleted(FStateTreeExecutionContext& Context, const EStateTreeRunStatus CompletionStatus,
	const FStateTreeActiveStates& CompletedActiveStates) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	InstanceData.bFoundPath = false;
	InstanceData.QueryID = -1;
	
	FMassStateTreeTaskBase::StateCompleted(Context, CompletionStatus, CompletedActiveStates);
}
