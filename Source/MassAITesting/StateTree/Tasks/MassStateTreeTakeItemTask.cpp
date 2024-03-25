// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeTakeItemTask.h"

#include "MassEntitySubsystem.h"
#include "StateTreeLinker.h"
#include "MassAITesting/RTSBuildingSubsystem.h"
#include "MassAITesting/Mass/RTSItemTrait.h"

bool FMassStateTreeTakeItemTask::Link(FStateTreeLinker& Linker)
{
	//Linker.LinkInstanceDataProperty(EntityHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeTakeItemTaskInstanceData, ItemHandle));
	
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);
	return true;
}

EStateTreeRunStatus FMassStateTreeTakeItemTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	const FMassEntityHandle& ItemHandle = Context.GetInstanceData<FMassEntityHandle>(*this);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	URTSBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	
	if (EntitySubsystem.GetEntityManager().IsEntityValid(ItemHandle))
	{
		const FItemFragment* Item = EntitySubsystem.GetEntityManager().GetFragmentDataPtr<FItemFragment>(ItemHandle);
		BuildingSubsystem.ItemHashGrid.Remove(ItemHandle, Item->CellLoc);
		EntitySubsystem.GetEntityManager().Defer().DestroyEntity(ItemHandle);
		return EStateTreeRunStatus::Succeeded;
	}
	return EStateTreeRunStatus::Failed;
}
