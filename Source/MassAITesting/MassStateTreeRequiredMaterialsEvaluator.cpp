// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeRequiredMaterialsEvaluator.h"

#include "MassSmartObjectBehaviorDefinition.h"
#include "SmartObjectSubsystem.h"
#include "StateTreeExecutionContext.h"

void FMassStateTreeRequiredMaterialsEvaluator::Evaluate(FStateTreeExecutionContext& Context,
                                                        const EStateTreeEvaluationType EvalType, const float DeltaTime) const
{
	const FRTSAgentFragment& RTSAgent = Context.GetExternalData(RTSAgentHandle);
	TEnumAsByte<EResourceType>& ResourceType = Context.GetInstanceData(ResourceTypeHandle);
	FSmartObjectRequestFilter& Filter = Context.GetInstanceData(FilterHandle);
	bool& bNeedsResources = Context.GetInstanceData(NeedsResourcesHandle);

	Filter.BehaviorDefinitionClass = USmartObjectMassBehaviorDefinition::StaticClass();
	bNeedsResources = false;
	FGameplayTagQueryExpression Query;
	Query.AllTagsMatch();
	
	if (RTSAgent.RequiredResources.Contains(EResourceType::Rock))
	{
		Query.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Object.Rock")));
		ResourceType = Rock;
		bNeedsResources = true;
	}
	else if (RTSAgent.RequiredResources.Contains(EResourceType::Tree))
	{
		Query.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Object.Tree")));
		ResourceType = Tree;
		bNeedsResources = true;
	}
	else
	{
		Query.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Object.Home")));
	}
	Filter.ActivityRequirements.Build(Query);
}

bool FMassStateTreeRequiredMaterialsEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(RTSAgentHandle);

	Linker.LinkInstanceDataProperty(ResourceTypeHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, ResourceNeeded));
	Linker.LinkInstanceDataProperty(NeedsResourcesHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, bNeedsResources));
	Linker.LinkInstanceDataProperty(FilterHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeRequiredMaterialsEvaluatorInstanceData, Filter));

	return true;
}
