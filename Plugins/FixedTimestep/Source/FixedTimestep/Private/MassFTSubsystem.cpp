// Fill out your copyright notice in the Description page of Project Settings.


#include "MassFTSubsystem.h"

#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "MassSimulationSubsystem.h"
#include "Processors/MassFTCompositeProcessor.h"
#include "Processors/MassFTProcessor.h"
#include "Runtime/MassEntity/Public/MassProcessor.h"

float UMassFTSubsystem::GetInterpolationAlpha() const
{
	return AccumulatedDeltaTime / (1.f / 60.f);
}

void UMassFTSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	MassSim = Collection.InitializeDependency<UMassSimulationSubsystem>();
}

void UMassFTSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());
	
	FixedTimestepProcessors = NewObject<UMassFTCompositeProcessor>(EntityManager.GetOwner());
	FixedTimestepProcessors->SetGroupName("FixedTimestepTick");
	
	TArray<UClass*> ProcessorClasses;
	GetDerivedClasses(UMassFTProcessor::StaticClass(), ProcessorClasses, true);
	
	FMassExecutionRequirements ChildExecutionRequirements = FMassExecutionRequirements();
	
	// Register derived classes
	TArray<UMassProcessor*> Processors;
	for (const UClass* ProcessorClass : ProcessorClasses)
	{
		UMassFTProcessor* Processor = NewObject<UMassFTProcessor>(EntityManager.GetOwner(), ProcessorClass);
		Processor->CallInitialize(EntityManager.GetOwner(), EntityManager.AsShared());
		
		Processor->ExportRequirements(ChildExecutionRequirements);
		Processors.Emplace(Processor);
	}
	
	// Composite processor requirements are equal to all the child processor requirements
	FixedTimestepProcessors->SetChildProcessors(Processors);
	FixedTimestepProcessors->SetProcessors(Processors, EntityManager.AsShared());
	
	FixedTimestepProcessors->CallInitialize(EntityManager.GetOwner(), EntityManager.AsShared());
	
	MassSim->RegisterDynamicProcessor(*FixedTimestepProcessors);
}

void UMassFTSubsystem::TickProcessors(float DeltaTime)
{
	AccumulatedDeltaTime += DeltaTime;
	
	const float FixedStep = 1.0f / 60.0f;
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());
	
	// Cap delta time to prevent endless spiral
	AccumulatedDeltaTime = FMath::Min(AccumulatedDeltaTime, FixedStep * 4.0f); 

	if (!EntityManager.IsProcessing())
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FixedTimestep)
		while (AccumulatedDeltaTime >= FixedStep)
		{
			// Flush commands gets executed when going out of scope
			FMassProcessingContext Context(EntityManager, FixedStep);
        
			// Execute the entire group of processors in order
			auto GraphRef = UE::Mass::Executor::TriggerParallelTasks(*FixedTimestepProcessors, MoveTemp(Context), []()
			{
				
			});
        
			AccumulatedDeltaTime -= FixedStep;
		}
	}
}
