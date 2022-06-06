// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "RTSAgentTrait.h"
#include "MassStateTreeTypes.h"
#include "SmartObjectSubsystem.h"
#include "MassStateTreeRequiredMaterialsEvaluator.generated.h"

USTRUCT()
struct MASSAITESTING_API FMassStateTreeRequiredMaterialsEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
	TEnumAsByte<EResourceType> ResourceNeeded = Tree;

	UPROPERTY(EditAnywhere, Category = Output)
	FSmartObjectRequestFilter Filter;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bNeedsResources = false;
};

/**
 * 
 */
USTRUCT()
struct MASSAITESTING_API FMassStateTreeRequiredMaterialsEvaluator : public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()

	virtual void Evaluate(FStateTreeExecutionContext& Context, const EStateTreeEvaluationType EvalType, const float DeltaTime) const override;
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeRequiredMaterialsEvaluatorInstanceData::StaticStruct(); }

	TStateTreeExternalDataHandle<FRTSAgentFragment> RTSAgentHandle;
	
	TStateTreeInstanceDataPropertyHandle<TEnumAsByte<EResourceType>> ResourceTypeHandle;
	TStateTreeInstanceDataPropertyHandle<bool> NeedsResourcesHandle;
	TStateTreeInstanceDataPropertyHandle<FSmartObjectRequestFilter> FilterHandle;
};
