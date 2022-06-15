// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassSignalSubsystem.h"
#include "MassSmartObjectRequest.h"
#include "MassStateTreeTypes.h"
#include "RTSAgentTrait.h"
#include "SmartObjectSubsystem.h"
#include "MassStateTreeSmartObjectEvaluatorPlus.generated.h"

struct FMassSmartObjectUserFragment;
struct FTransformFragment;

USTRUCT()
struct MASSAITESTING_API FMassStateTreeSmartObjectEvaluatorPlusInstanceData
{
	GENERATED_BODY()

	/** The identifier of the search request send by the evaluator to find candidates */
	UPROPERTY(EditAnywhere, Category = Output)
	FMassSmartObjectRequestResult SearchRequestResult;

	/** Indicates that the result of the candidates search is ready and contains some candidates */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bCandidatesFound = false;

	UPROPERTY(EditAnywhere, Category = Parameter)
	FSmartObjectRequestFilter Filter = FSmartObjectRequestFilter();

	UPROPERTY(EditAnywhere, Category = Parameter)
	float Range = 5000.f;
};

/**
 * 
 */
USTRUCT()
struct MASSAITESTING_API FMassStateTreeSmartObjectEvaluatorPlus : public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()

	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeSmartObjectEvaluatorPlusInstanceData::StaticStruct(); }
	virtual void ExitState(FStateTreeExecutionContext& Context, const EStateTreeStateChangeType ChangeType, const FStateTreeTransitionResult& Transition) const override;
	virtual void Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const override;
	void Reset(FStateTreeExecutionContext& Context) const;

	TStateTreeInstanceDataPropertyHandle<FMassSmartObjectRequestResult> SearchRequestResultHandle;
	TStateTreeInstanceDataPropertyHandle<bool> CandidatesFoundHandle;
	TStateTreeInstanceDataPropertyHandle<FSmartObjectRequestFilter> FilterHandle;
	TStateTreeInstanceDataPropertyHandle<float> RangeHandle;
	
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<FMassSmartObjectUserFragment> SmartObjectUserHandle;
	TStateTreeExternalDataHandle<FRTSAgentFragment> RTSAgentHandle;
};
