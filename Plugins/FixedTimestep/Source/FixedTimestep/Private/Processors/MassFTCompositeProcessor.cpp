// Fill out your copyright notice in the Description page of Project Settings.


#include "Processors/MassFTCompositeProcessor.h"

#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Data/FTFragments.h"

UMassFTCompositeProcessor::UMassFTCompositeProcessor()
{
	QueryBasedPruning = EMassQueryBasedPruning::Never;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::FixedTimestep;
}

FGraphEventRef UMassFTCompositeProcessor::DispatchProcessorTasks(const TSharedPtr<FMassEntityManager>& EntityManager,
	FMassExecutionContext& ExecutionContext, const FGraphEventArray& Prerequisites)
{
	FGraphEventArray Events;
	FGraphEventRef PreviousEvent = FGraphEventRef();
	
	FMassExecutionContext Context = GetFixedTimestepContext(ExecutionContext);
	SubstepProcessors(ExecutionContext.GetDeltaTimeSeconds(), [&]()
	{
		FGraphEventRef Event = Super::DispatchProcessorTasks(EntityManager, Context, Prerequisites);
		if (PreviousEvent.IsValid())
		{
			Event->AddPrerequisites(PreviousEvent);
		}
		
		Events.Add(Event);
	});
	
	FGraphEventRef CompletionEvent = FFunctionGraphTask::CreateAndDispatchWhenReady([this](){}
		, TStatId{}, &Events, ENamedThreads::AnyHiPriThreadHiPriTask);
	
	return CompletionEvent;
}

void UMassFTCompositeProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	SubstepProcessors(Context.GetDeltaTimeSeconds(), [&]()
	{
		FMassExecutionContext FixedStepContext = GetFixedTimestepContext(Context);
		Super::Execute(EntityManager, FixedStepContext);
	});
}

void UMassFTCompositeProcessor::SubstepProcessors(float DeltaTime, const TFunction<void()>& SubstepFunction)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FixedTimestep)
	
	AccumulatedDeltaTime += DeltaTime;
	
	const float FixedStep = 1.0f / 60.0f;
	
	// Cap delta time to prevent endless spiral
	AccumulatedDeltaTime = FMath::Min(AccumulatedDeltaTime, FixedStep * 4.0f); 
	
	while (AccumulatedDeltaTime >= FixedStep)
	{
		SubstepFunction();
		AccumulatedDeltaTime -= FixedStep;
	}
}

FMassExecutionContext UMassFTCompositeProcessor::GetFixedTimestepContext(
	const FMassExecutionContext& InExecutionContext)
{
	const float FixedStep = 1.0f / 60.0f;
	FMassExecutionContext OutExecutionContext = FMassExecutionContext(InExecutionContext.GetEntityManagerChecked(), FixedStep);
	
	// A bit of jank here because we cannot change DeltaTime after it's been set in the constructor
	// We are essentially simulating FProcessingContext::GetExecutionContext()
	OutExecutionContext.SetFlushDeferredCommands(false);
	OutExecutionContext.SetDeferredCommandBuffer(MakeShareable(new FMassCommandBuffer()));
	OutExecutionContext.SetExecutionType(EMassExecutionContextType::Processor);
	
	return OutExecutionContext;
}
