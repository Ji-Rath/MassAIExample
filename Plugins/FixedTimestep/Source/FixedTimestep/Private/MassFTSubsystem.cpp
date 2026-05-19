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
	MassSim->GetOnProcessingPhaseStarted(EMassProcessingPhase::PrePhysics).AddUObject(this, &UMassFTSubsystem::TickProcessors);
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
}

void UMassFTSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//TickProcessors(DeltaTime);
}

TStatId UMassFTSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UMassFTSubsystem, STATGROUP_Tickables);
}

void UMassFTSubsystem::TickProcessors(float DeltaTime)
{
	if (!FixedTimestepProcessors) { return; }
	
	AccumulatedDeltaTime += DeltaTime;
	
	const float FixedStep = 1.0f / 60.0f;
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());
	
	// Cap delta time to prevent endless spiral
	AccumulatedDeltaTime = FMath::Min(AccumulatedDeltaTime, FixedStep * 4.0f); 

	if (!EntityManager.IsProcessing())
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FixedTimestep)
		
		// Flush commands gets executed when going out of scope
		FMassProcessingContext Context(EntityManager, FixedStep);
		
		while (AccumulatedDeltaTime >= FixedStep)
		{
			// Execute the entire group of processors in order
			FMassExecutionContext ExecutionContext = MoveTemp(Context).GetExecutionContext();
			FGraphEventRef GraphRef = FixedTimestepProcessors->DispatchProcessorTasks(Context.GetEntityManager(), ExecutionContext);
			GraphRef->Wait();
			
			AccumulatedDeltaTime -= FixedStep;
		}
	}
}
