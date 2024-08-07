// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTree/Evaluators/MassResourceUserPropertyEvaluator.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Mass/ResourceEntity.h"

void FMassResourceUserPropertyEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const FResourceUserFragment& ResourceUserFragment = Context.GetExternalData(ResourceUserFragmentHandle);

	InstanceData.UserTags = ResourceUserFragment.Tags;
}

bool FMassResourceUserPropertyEvaluator::Link(FStateTreeLinker& Linker)
{
	Linker.LinkExternalData(ResourceUserFragmentHandle);
	return true;
}
