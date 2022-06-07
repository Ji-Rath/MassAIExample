// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeRequiredMaterialsEvaluator.h"

#include "MassSmartObjectBehaviorDefinition.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"

void FMassStateTreeRequiredMaterialsEvaluator::Evaluate(FStateTreeExecutionContext& Context,
                                                        const EStateTreeEvaluationType EvalType, const float DeltaTime) const
{
	const FRTSAgentFragment& RTSAgent = Context.GetExternalData(RTSAgentHandle);
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	FTransform& Transform = Context.GetExternalData(TransformHandle).GetMutableTransform();
	
	TEnumAsByte<EResourceType>& ResourceType = Context.GetInstanceData(ResourceTypeHandle);
	FSmartObjectRequestFilter& Filter = Context.GetInstanceData(FilterHandle);
	bool& bNeedsResources = Context.GetInstanceData(NeedsResourcesHandle);

	Filter.BehaviorDefinitionClass = USmartObjectMassBehaviorDefinition::StaticClass();
	bNeedsResources = false;

	FSmartObjectRequest Request;
	Request.QueryBox = FBox::BuildAABB(Transform.GetLocation(), FVector(5000.f));

	// Empty query, to setup for return filter
	FGameplayTagQueryExpression Query;
	Query.AllTagsMatch();

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
			bNeedsResources = true;
			break;
		}
	}

	if (!bNeedsResources)
		Query.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Object.Home")));

	Filter.ActivityRequirements.Build(Query);
}

bool FMassStateTreeRequiredMaterialsEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(RTSAgentHandle);
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	Linker.LinkExternalData(TransformHandle);

	Linker.LinkInstanceDataProperty(ResourceTypeHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, ResourceNeeded));
	Linker.LinkInstanceDataProperty(NeedsResourcesHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, bNeedsResources));
	Linker.LinkInstanceDataProperty(FilterHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, Filter));

	return true;
}
