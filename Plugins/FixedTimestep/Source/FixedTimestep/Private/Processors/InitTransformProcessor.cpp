// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/InitTransformProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "Data/FTFragments.h"

UInitTransformProcessor::UInitTransformProcessor() : EntityQuery(*this)
{
	ObservedOperations = EMassObservedOperationFlags::Add;
	ObservedType = FFTTransformFragment::StaticStruct();
}

void UInitTransformProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FFTTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
}

void UInitTransformProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		auto FTTransformFragments = Context.GetMutableFragmentView<FFTTransformFragment>();
		auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
		
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTTransformFragments[EntityIt].GetMutableTransform() = TransformFragments[EntityIt].GetTransform();
		}
	});
}
