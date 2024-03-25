// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassSignalSubsystem.h"
#include "MassSmartObjectRequest.h"
#include "MassStateTreeTypes.h"
#include "SmartObjectSubsystem.h"
#include "MassStateTreeSmartObjectEvaluatorPlus.generated.h"

struct FRTSAgentFragment;
struct FMassSmartObjectUserFragment;
struct FTransformFragment;

USTRUCT()
struct MASSAITESTING_API FMassStateTreeSmartObjectEvaluatorPlusInstanceData
{
	GENERATED_BODY()

	/** The identifier of the search request send by the evaluator to find candidates */
	UPROPERTY(EditAnywhere, Category = Output)
	FMassSmartObjectRequestResultFragment SearchRequestResult;

	/** Indicates that the result of the candidates search is ready and contains some candidates */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bCandidatesFound = false;

	UPROPERTY(EditAnywhere, Category = Input)
	FSmartObjectRequestFilter Filter;

	UPROPERTY(EditAnywhere, Category = Input)
	FSmartObjectHandle SOHandle;

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

	using FInstanceDataType = FMassStateTreeSmartObjectEvaluatorPlusInstanceData;
	
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeSmartObjectEvaluatorPlusInstanceData::StaticStruct(); }
	virtual void TreeStop(FStateTreeExecutionContext& Context) const override;
	//virtual void Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	void Reset(FStateTreeExecutionContext& Context) const;
	
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
	TStateTreeExternalDataHandle<FTransformFragment> EntityTransformHandle;
	TStateTreeExternalDataHandle<FMassSmartObjectUserFragment> SmartObjectUserHandle;
	TStateTreeExternalDataHandle<FRTSAgentFragment> RTSAgentHandle;
};
