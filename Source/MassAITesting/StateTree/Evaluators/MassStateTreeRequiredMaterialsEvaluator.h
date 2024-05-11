// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassCommonFragments.h"
#include "MassStateTreeTypes.h"
#include "SmartObjectSubsystem.h"
#include "MassStateTreeRequiredMaterialsEvaluator.generated.h"

class UMassSignalSubsystem;
class UMassEntitySubsystem;
class URTSBuildingSubsystem;
struct FRTSAgentFragment;
USTRUCT()
struct MASSAITESTING_API FMassStateTreeRequiredMaterialsEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = Output)
	FSmartObjectRequestFilter Filter;

	UPROPERTY(EditAnywhere, Category = Output)
	FSmartObjectHandle SmartObjectHandle;

	UPROPERTY(EditAnywhere, Category = Output)
	FMassEntityHandle ItemHandle;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bFoundSmartObject = false;

	UPROPERTY(EditAnywhere, Category = Output)
	bool bFoundItemHandle = false;
};

/**
 * 
 */
USTRUCT()
struct MASSAITESTING_API FMassStateTreeRequiredMaterialsEvaluator : public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()

	using FInstanceDataType = FMassStateTreeRequiredMaterialsEvaluatorInstanceData;
	
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassStateTreeRequiredMaterialsEvaluatorInstanceData::StaticStruct(); }

	TStateTreeExternalDataHandle<FRTSAgentFragment> RTSAgentHandle;
	TStateTreeExternalDataHandle<USmartObjectSubsystem> SmartObjectSubsystemHandle;
	TStateTreeExternalDataHandle<UMassEntitySubsystem> EntitySubsystemHandle;
	TStateTreeExternalDataHandle<FTransformFragment> TransformHandle;
	TStateTreeExternalDataHandle<URTSBuildingSubsystem> BuildingSubsystemHandle;
	TStateTreeExternalDataHandle<UMassSignalSubsystem> MassSignalSubsystemHandle;
};
