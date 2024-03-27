// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeRequiredMaterialsEvaluator.h"

#include "MassEntitySubsystem.h"
#include "MassSmartObjectBehaviorDefinition.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassAITesting/RTSBuildingSubsystem.h"
#include "MassAITesting/Mass/RTSItemTrait.h"

void FMassStateTreeRequiredMaterialsEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("RequiredMaterialsEvaluator"));

	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	FRTSAgentFragment& RTSAgent = Context.GetExternalData(RTSAgentHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	URTSBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	const FVector& Location = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();
	
	FSmartObjectRequestFilter& Filter = InstanceData.Filter;
	bool& bFoundSmartObject = InstanceData.bFoundSmartObject;
	bool& bFoundItemHandle = InstanceData.bFoundItemHandle;
	FSmartObjectHandle& SOHandle = InstanceData.SmartObjectHandle;
	FMassEntityHandle& EntityHandle = InstanceData.ItemHandle;

	bFoundSmartObject = false;
	bFoundItemHandle = false;

	Filter.BehaviorDefinitionClass = USmartObjectMassBehaviorDefinition::StaticClass();

	// Basic setup
	// This evaluator should simply gather data for the state tree to evaluate
	// In terms of priority, the ai should follow these rules
	// 1. If there is a rock/wood pair and a floor to be built, claim floor and collect the items. Then build the floor
	// 2. If there is a queued resource to be chopped, chop it down
	// Outputs:
	// - smart object filter, item handle
	// - AgentState: Chopping Resources/Gathering Item/Building Floor

	// We are currently gathering resources
	if (!EntitySubsystem.GetEntityManager().IsEntityValid(EntityHandle) && RTSAgent.QueuedItems.Num() > 0)
	{
		EntityHandle = RTSAgent.QueuedItems.Pop();
		if (EntitySubsystem.GetEntityManager().IsEntityValid(EntityHandle))
		{
			bFoundItemHandle = true;
			FItemFragment* ItemFragment = EntitySubsystem.GetEntityManager().GetFragmentDataPtr<FItemFragment>(EntityHandle);
			if (ItemFragment)
			{
				ItemFragment->bClaimed = true;
			}
			return;
		}
	}

	
	// Check whether agent is waiting for a command
	if (RTSAgent.QueuedItems.Num() <= 0 && !RTSAgent.BuildingHandle.IsValid())
	{
		// Before calculating items existence, we need to see if a building needs building
		if (BuildingSubsystem.GetQueuedBuildings() > 0)
		{
			// Before giving commands, we need to make sure the item(s) are available
			TArray<FMassEntityHandle> ItemHandles;
			ItemHandles.AddUninitialized(2);
			
			if (BuildingSubsystem.FindItem(Location, 5000.f, Rock, ItemHandles[0]))
			{
				if (BuildingSubsystem.FindItem(Location, 5000.f, Tree, ItemHandles[1]))
				{
					bFoundItemHandle = true;
					// Since they are available, we can claim/give the agent the handles to fetch them
					BuildingSubsystem.ClaimFloor(RTSAgent.BuildingHandle);
					
					RTSAgent.QueuedItems.Append(ItemHandles);
					EntityHandle = RTSAgent.QueuedItems.Pop();
					
					// We need to claim both because then its possible that another agent also 'wants' the item on the ground
					for(const FMassEntityHandle& Item : ItemHandles)
					{
						if (FItemFragment* ItemFragment = EntitySubsystem.GetEntityManager().GetFragmentDataPtr<FItemFragment>(Item))
						{
							ItemFragment->bClaimed = true;
						}
					}
					
					return;
				}
			}
		}
	}
	
	
	// We have finished collecting items and should head back to our building
	if(RTSAgent.BuildingHandle.IsValid() && RTSAgent.QueuedItems.Num() == 0)
	{
		bFoundSmartObject = true;
		SOHandle = RTSAgent.BuildingHandle;
		return;
	}

	// We dont have the resources/dont have a floor to build, so check if there are queued resources to chop
	if(BuildingSubsystem.GetNumQueuedResources() > 0 && !RTSAgent.ResourceHandle.IsValid())
	{
		// We have queued resources to chop, so we should try to chop one
		FSmartObjectHandle ResourceHandle;
		bFoundSmartObject = true;
		BuildingSubsystem.ClaimResource(ResourceHandle);
		RTSAgent.ResourceHandle = ResourceHandle;
		SOHandle = ResourceHandle;
	}
}

bool FMassStateTreeRequiredMaterialsEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(RTSAgentHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);

	return true;
}
