// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeSmartObjectEvaluatorPlus.h"

#include "MassAIBehaviorTypes.h"
#include "MassCommonFragments.h"
#include "MassSmartObjectFragments.h"
#include "StateTreeExecutionContext.h"
#include "MassAITesting/Mass/RTSAgentTrait.h"

bool FMassStateTreeSmartObjectEvaluatorPlus::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(SmartObjectUserHandle);
	Linker.LinkExternalData(RTSAgentHandle);

	Linker.LinkInstanceDataProperty(SmartObjectHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeSmartObjectEvaluatorPlusInstanceData, SOHandle));
	Linker.LinkInstanceDataProperty(CandidatesFoundHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeSmartObjectEvaluatorPlusInstanceData, bCandidatesFound));
	Linker.LinkInstanceDataProperty(RangeHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeSmartObjectEvaluatorPlusInstanceData, Range));
	Linker.LinkInstanceDataProperty(SmartObjectHandle, STATETREE_INSTANCEDATA_PROPERTY(FMassStateTreeSmartObjectEvaluatorPlusInstanceData, SOHandle));

	return true;
}

void FMassStateTreeSmartObjectEvaluatorPlus::ExitState(FStateTreeExecutionContext& Context,
	const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const
{
	if (ChangeType != EStateTreeStateChangeType::Changed)
	{
		return;
	}

	Reset(Context);
}

void FMassStateTreeSmartObjectEvaluatorPlus::Evaluate(FStateTreeExecutionContext& Context,
	const EStateTreeEvaluationType EvalType, const float DeltaTime) const
{
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	FMassSmartObjectUserFragment& SOUser = Context.GetExternalData(SmartObjectUserHandle);
	FRTSAgentFragment& RTSAgent = Context.GetExternalData(RTSAgentHandle);

	FMassSmartObjectRequestResult& RequestResult = Context.GetInstanceData(SearchRequestResultHandle);
	FSmartObjectRequestFilter& Filter = Context.GetInstanceData(FilterHandle);
	FSmartObjectHandle& SOHandle = Context.GetInstanceData(SmartObjectHandle);
	bool& CandidatesFound = Context.GetInstanceData(CandidatesFoundHandle);
	float& Range = Context.GetInstanceData(RangeHandle);

	const FTransform& Transform = TransformFragment.GetTransform();

	bool bClaimed = SOUser.ClaimHandle.IsValid();

	// We are returning to our claimed floor home, we dont need to perform any searching.
	FGameplayTagQueryExpression Expression;
	Filter.ActivityRequirements.GetQueryExpr(Expression);
	if (RTSAgent.BuildingHandle.IsValid() && Expression.TagSet.Contains(FGameplayTag::RequestGameplayTag(TEXT("Object.Home"))))
	{
		RequestResult.Candidates[0] = FSmartObjectCandidate(RTSAgent.BuildingHandle, 0);
		RequestResult.NumCandidates++;
		RequestResult.bProcessed = true;
		CandidatesFound = true;
		
		return;
	}

	// Already claimed, nothing to do
	if (bClaimed)
	{
		return;
	}
	
	FSmartObjectRequest Request;
	Request.Filter = Filter;
	Request.QueryBox = FBox::BuildAABB(Transform.GetLocation(), FVector(Range));
	FSmartObjectRequestResult Result = SmartObjectSubsystem.FindSmartObject(Request);
	if (Result.IsValid())
	{
		RequestResult.Candidates[0] = FSmartObjectCandidate(Result.SmartObjectHandle, 0);
		RequestResult.NumCandidates++;
		RequestResult.bProcessed = true;
		CandidatesFound = true;
	}
}

void FMassStateTreeSmartObjectEvaluatorPlus::Reset(FStateTreeExecutionContext& Context) const
{
	bool& bCandidatesFound = Context.GetInstanceData(CandidatesFoundHandle);
	FMassSmartObjectRequestResult& RequestResult = Context.GetInstanceData(SearchRequestResultHandle);
	bCandidatesFound = false;
	RequestResult.bProcessed = false;
	RequestResult.NumCandidates = 0;
}
