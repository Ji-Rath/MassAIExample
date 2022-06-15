// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeRequiredMaterialsEvaluator.h"

#include "BuildingFragments.h"
#include "MassSmartObjectBehaviorDefinition.h"
#include "MassStateTreeExecutionContext.h"
#include "RTSBuildingSubsystem.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"

void FMassStateTreeRequiredMaterialsEvaluator::Evaluate(FStateTreeExecutionContext& Context,
                                                        const EStateTreeEvaluationType EvalType, const float DeltaTime) const
{
	const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
	
	FRTSAgentFragment& RTSAgent = Context.GetExternalData(RTSAgentHandle);
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	UMassEntitySubsystem& EntitySubsystem = Context.GetExternalData(EntitySubsystemHandle);
	URTSBuildingSubsystem& BuildingSubsystem = Context.GetExternalData(BuildingSubsystemHandle);
	FTransform& Transform = Context.GetExternalData(TransformHandle).GetMutableTransform();
	
	TEnumAsByte<EResourceType>& ResourceType = Context.GetInstanceData(ResourceTypeHandle);
	FSmartObjectRequestFilter& Filter = Context.GetInstanceData(FilterHandle);
	bool& bFoundSmartObjectFilter = Context.GetInstanceData(FoundSmartObjectFilterHandle);
	bFoundSmartObjectFilter = false;

	Filter.BehaviorDefinitionClass = USmartObjectMassBehaviorDefinition::StaticClass();

	FSmartObjectRequest Request;
	Request.QueryBox = FBox::BuildAABB(Transform.GetLocation(), FVector(5000.f));

	// Empty query, to setup for return filter
	FGameplayTagQueryExpression Query;
	Query.AllTagsMatch();

	// Check to see if the entity has any resources they need to gather
	for(const TPair<EResourceType, int> Resource : RTSAgent.RequiredResources)
	{
		Query.TagSet.Empty();
		FName Tag = Resource.Key == Rock ? TEXT("Object.Rock") : TEXT("Object.Tree");

		//Test for resource
		Query.AddTag(Tag);
		Filter.ActivityRequirements.Build(Query);
		Request.Filter = Filter;
		FSmartObjectRequestResult Result = SmartObjectSubsystem.FindSmartObject(Request);
		if (Result.IsValid())
		{
			ResourceType = Resource.Key;
			bFoundSmartObjectFilter = true;
			break;
		}
	}

	// If the entity doesnt need to collect any resources then there are two options
	// 1. The entity has gathered all their required resources and should return to build the floor
	// 2. The entity has no resources and should find a job to do
	// todo I should probably separate these tasks to different nodes since they perform different functions
	if (!bFoundSmartObjectFilter)
	{
		if (RTSAgent.BuildingHandle.IsValid())
		{
			if (RTSAgent.RequiredResources.Num() == 0)
			{
				Query.TagSet.Empty();
				Query.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Object.Home")));
				bFoundSmartObjectFilter = true;
			}
		}
		else
		{
			// Building found with needed construction, give entity required resources
			FSmartObjectHandle BuildingHandle;
			BuildingSubsystem.ClaimFloor(OUT BuildingHandle);
			if (BuildingHandle.IsValid())
			{
				//EntitySubsystem.Defer().AddTag<FRTSRequestResources>(MassContext.GetEntity());
				RTSAgent.RequiredResources.Emplace(Rock, 1);
				RTSAgent.RequiredResources.Emplace(Tree, 1);
				RTSAgent.BuildingHandle = BuildingHandle;
			}
			bFoundSmartObjectFilter = false;
		}
	}

	Filter.ActivityRequirements.Build(Query);
}

bool FMassStateTreeRequiredMaterialsEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(RTSAgentHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	Linker.LinkExternalData(TransformHandle);
	Linker.LinkExternalData(EntitySubsystemHandle);
	Linker.LinkExternalData(BuildingSubsystemHandle);

	Linker.LinkInstanceDataProperty(ResourceTypeHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, ResourceNeeded));
	Linker.LinkInstanceDataProperty(FoundSmartObjectFilterHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, bFoundSmartObjectFilter));
	Linker.LinkInstanceDataProperty(FilterHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, Filter));

	return true;
}
