// Fill out your copyright notice in the Description page of Project Settings.


#include "MassStateTreeSmartObjectEvaluatorPlus.h"

#include "MassAIBehaviorTypes.h"
#include "MassCommonFragments.h"
#include "MassSmartObjectFragments.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "MassAITesting/Mass/RTSAgentTrait.h"

bool FMassStateTreeSmartObjectEvaluatorPlus::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(SmartObjectSubsystemHandle);
	Linker.LinkExternalData(MassSignalSubsystemHandle);
	Linker.LinkExternalData(EntityTransformHandle);
	Linker.LinkExternalData(SmartObjectUserHandle);
	Linker.LinkExternalData(RTSAgentHandle);

	return true;
}

void FMassStateTreeSmartObjectEvaluatorPlus::TreeStop(FStateTreeExecutionContext& Context) const
{
	Reset(Context);
}

void FMassStateTreeSmartObjectEvaluatorPlus::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	
	USmartObjectSubsystem& SmartObjectSubsystem = Context.GetExternalData(SmartObjectSubsystemHandle);
	FTransformFragment& TransformFragment = Context.GetExternalData(EntityTransformHandle);
	UMassSignalSubsystem& SignalSubsystem = Context.GetExternalData(MassSignalSubsystemHandle);
	FMassSmartObjectUserFragment& SOUser = Context.GetExternalData(SmartObjectUserHandle);
	FRTSAgentFragment& RTSAgent = Context.GetExternalData(RTSAgentHandle);

	FMassSmartObjectRequestResultFragment& RequestResult = InstanceData.SearchRequestResult;
	FSmartObjectRequestFilter& Filter = InstanceData.Filter;
	FSmartObjectHandle& SOHandle = InstanceData.SOHandle;
	bool& CandidatesFound = InstanceData.bCandidatesFound;
	float& Range = InstanceData.Range;

	const FTransform& Transform = TransformFragment.GetTransform();

	bool bClaimed = SOUser.InteractionHandle.IsValid();

	// We are returning to our claimed floor home, we dont need to perform any searching.
	FGameplayTagQueryExpression Expression;
	Filter.ActivityRequirements.GetQueryExpr(Expression);
	if (RTSAgent.BuildingHandle.IsValid() && Expression.TagSet.Contains(FGameplayTag::RequestGameplayTag(TEXT("Object.Home"))))
	{
		//RequestResult.Candidates[0] = FSmartObjectCandidate(RTSAgent.BuildingHandle, 0);
		//RequestResult.NumCandidates++;
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
		//RequestResult.Candidates[0] = FSmartObjectCandidate(Result.SmartObjectHandle, 0);
		//RequestResult.NumCandidates++;
		RequestResult.bProcessed = true;
		CandidatesFound = true;
	}
}

void FMassStateTreeSmartObjectEvaluatorPlus::Reset(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	bool& bCandidatesFound = InstanceData.bCandidatesFound;
	FMassSmartObjectRequestResultFragment& RequestResult = InstanceData.SearchRequestResult;
	bCandidatesFound = false;
	RequestResult.bProcessed = false;
	//RequestResult.NumCandidates = 0;
}
