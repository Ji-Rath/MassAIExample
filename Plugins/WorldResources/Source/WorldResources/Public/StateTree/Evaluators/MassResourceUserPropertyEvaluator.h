// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassStateTreeTypes.h"
#include "Mass/ResourceEntity.h"
#include "MassResourceUserPropertyEvaluator.generated.h"

struct FOpenSimulationFragment;

USTRUCT()
struct WORLDRESOURCES_API FMassResourceUserPropertyEvaluatorInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Output)
	FGameplayTagContainer UserTags;
};

USTRUCT(meta = (DisplayName = "Mass Resource User Property Evaluator"))
struct FMassResourceUserPropertyEvaluator : public FMassStateTreeEvaluatorBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FMassResourceUserPropertyEvaluatorInstanceData;
		
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual bool Link(FStateTreeLinker& Linker) override;
	virtual const UStruct* GetInstanceDataType() const override { return FMassResourceUserPropertyEvaluatorInstanceData::StaticStruct(); }

protected:
	TStateTreeExternalDataHandle<FResourceUserFragment> ResourceUserFragmentHandle;
};
