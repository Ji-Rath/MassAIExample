// Fill out your copyright notice in the Description page of Project Settings.


#include "MassFindEmptySpotTask.h"

#include "MassCommonFragments.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MassAITesting/Subsystems/GridManagerSubsystem.h"

bool FMassFindEmptySpotTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(GridManagerHandle);
	Linker.LinkExternalData(EntityTransformHandle);
	return true;
}

EStateTreeRunStatus FMassFindEmptySpotTask::EnterState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	auto& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	auto& GridManagerSubsystem = Context.GetExternalData(GridManagerHandle);

	TArray<int32> NearbyNodes;
	GridManagerSubsystem.GetNearbyNodes(TransformFragment.GetTransform().GetLocation(), NearbyNodes, 2000.f);

	NearbyNodes.Sort([&TransformFragment, &GridManagerSubsystem](const int32& Result, const int32& Result2)
	{
		FTransform SlotTransform, SlotTransform2;
		GridManagerSubsystem.GridMesh->GetInstanceTransform(Result, SlotTransform, true);
		GridManagerSubsystem.GridMesh->GetInstanceTransform(Result2, SlotTransform2, true);
		
		return FVector::DistSquared2D(SlotTransform.GetLocation(), TransformFragment.GetTransform().GetLocation()) < FVector::DistSquared2D(SlotTransform2.GetLocation(), TransformFragment.GetTransform().GetLocation());
	});

	for (int32 NearbyNode : NearbyNodes)
	{
		if (!GridManagerSubsystem.ClaimedNodes.Contains(NearbyNode))
		{
			GridManagerSubsystem.ClaimedNodes.Add(NearbyNode);
			
			FTransform NodeTransform;
			GridManagerSubsystem.GridMesh->GetInstanceTransform(NearbyNode, NodeTransform, true);
			InstanceData.EmptySpot = NodeTransform.GetLocation();
			return EStateTreeRunStatus::Running;
		}
	}
	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FMassFindEmptySpotTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	return FMassStateTreeTaskBase::Tick(Context, DeltaTime);
}

void FMassFindEmptySpotTask::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FMassStateTreeTaskBase::ExitState(Context, Transition);
}
