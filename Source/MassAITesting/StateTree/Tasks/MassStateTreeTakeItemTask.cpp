// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeTakeItemTask.h"

#include "MassAITesting/RTSBuildingSubsystem.h"
#include "MassAITesting/Mass/RTSItemTrait.h"

bool FMassStateTreeTakeItemTask::Link(FStateTreeLinker& Linker)
{
	Linker.LinkInstanceDataProperty(EntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeTakeItemTaskInstanceData, ItemHandle));
	
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMassStateTreeTakeItemTask::EnterState(FStateTreeExecutionContext& Context,
	const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	const FMassEntityHandle& ItemHandle = Context.GetInstanceData(EntityHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	URTSBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	
	if (EntitySubsystem.IsEntityValid(ItemHandle))
	{
		const FItemFragment* Item = EntitySubsystem.GetFragmentDataPtr<FItemFragment>(ItemHandle);
		BuildingSubsystem.ItemHashGrid.Remove(ItemHandle, Item->CellLoc);
		EntitySubsystem.Defer().DestroyEntity(ItemHandle);
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Failed;
}
