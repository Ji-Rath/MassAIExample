// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeRequiredMaterialsEvaluator.h"

#include "MassSmartObjectBehaviorDefinition.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"
#include "MassAITesting/RTSBuildingSubsystem.h"
#include "MassAITesting/Mass/RTSItemTrait.h"

void FMassStateTreeRequiredMaterialsEvaluator::Evaluate(FStateTreeExecutionContext& Context,
                                                        const EStateTreeEvaluationType EvalType, const float DeltaTime) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("RequiredMaterialsEvaluator"));
	// Since this eval does stuff thats not just...evaluating we need to make sure that it only gets called in the PreSelect
	// to avoid unintended results.
	// @todo move task-related logic to their own state tree tasks
	if (EvalType == EStateTreeEvaluationType::Tick)
		return;
	
	FRTSAgentFragment& RTSAgent = Context.GetExternalData(RTSAgentHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	URTSBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	const FVector& Location = Context.GetExternalData(TransformHandle).GetTransform().GetLocation();
	
	FSmartObjectRequestFilter& Filter = Context.GetInstanceData(FilterHandle);
	bool& bFoundSmartObject = Context.GetInstanceData(FoundSmartObjectHandle);
	bool& bFoundItemHandle = Context.GetInstanceData(FoundItemHandle);
	FSmartObjectHandle& SOHandle = Context.GetInstanceData(SmartObjectHandle);
	FMassEntityHandle& EntityHandle = Context.GetInstanceData(ItemHandle);

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
	if (!EntitySubsystem.IsEntityValid(EntityHandle) && RTSAgent.QueuedItems.Num() > 0)
	{
		EntityHandle = RTSAgent.QueuedItems.Pop();
		if (EntitySubsystem.IsEntityValid(EntityHandle))
		{
			bFoundItemHandle = true;
			FItemFragment* ItemFragment = EntitySubsystem.GetFragmentDataPtr<FItemFragment>(EntityHandle);
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
						if (FItemFragment* ItemFragment = EntitySubsystem.GetFragmentDataPtr<FItemFragment>(Item))
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
	
	Linker.LinkInstanceDataProperty(FoundSmartObjectHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, bFoundSmartObject));
	Linker.LinkInstanceDataProperty(FilterHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, Filter));
	Linker.LinkInstanceDataProperty(FoundItemHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, bFoundItemHandle));
	Linker.LinkInstanceDataProperty(SmartObjectHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, SmartObjectHandle));
	Linker.LinkInstanceDataProperty(ItemHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, ItemHandle));

	return true;
}
