// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassNavigationFragments.h"
#include "MassObserverRegistry.h"
#include "MassSignalProcessorBase.h"
#include "RTSMovementSubsystem.h"
#include "SmartObjectComponent.h"
#include "SmartObjectSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"

//----------------------------------------------------------------------//
// URTSGatherResourceProcessor
//----------------------------------------------------------------------//
void URTSGatherResourceProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FRTSGatherResourceFragment> GatherResourceFragments = Context.GetFragmentView<FRTSGatherResourceFragment>();
		const TArrayView<FRTSAgentFragment> RTSAgentFragment = Context.GetMutableFragmentView<FRTSAgentFragment>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSGatherResourceFragment& ResourceFragment = GatherResourceFragments[EntityIndex];
			FRTSAgentFragment& RTSAgent = RTSAgentFragment[EntityIndex];

			// Add the given resource to the agent
			int& InventoryItem = RTSAgent.Inventory.FindOrAdd(ResourceFragment.Resource);
			InventoryItem += ResourceFragment.Amount;

			// Subtract from required resources
			if (RTSAgent.RequiredResources.Contains(ResourceFragment.Resource))
			{
				InventoryItem = *RTSAgent.RequiredResources.Find(ResourceFragment.Resource);
				InventoryItem -= ResourceFragment.Amount;
				
				if (InventoryItem <= 0)
				{
					RTSAgent.RequiredResources.Remove(ResourceFragment.Resource);
				}
			}

			// Remove fragment so we dont infinitely grant resources
			// TODO: Consider using tags rather than just removing the fragment now
			Context.Defer().RemoveFragment<FRTSGatherResourceFragment>(Context.GetEntity(EntityIndex));
		}
	}));
}

void URTSGatherResourceProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSGatherResourceFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
}

void URTSGatherResourceProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

//----------------------------------------------------------------------//
// URTSAgentTrait
//----------------------------------------------------------------------//
void URTSAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);
	check(EntitySubsystem);
	
	BuildContext.AddFragment<FRTSAgentFragment>();
	BuildContext.AddTag<FRTSAgent>();
	BuildContext.AddTag<FRTSRequestResources>();
	
	const FConstSharedStruct RTSAgentFragment = EntitySubsystem->GetOrCreateConstSharedFragment(UE::StructUtils::GetStructCrc32(FConstStructView::Make(AgentParameters)), AgentParameters);
	BuildContext.AddConstSharedFragment(RTSAgentFragment);
}

//----------------------------------------------------------------------//
// URTSAgentInitializer
//----------------------------------------------------------------------//
URTSAgentInitializer::URTSAgentInitializer()
{
	//bAutoRegisterWithProcessingPhases = true;
	//ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	//ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
	ObservedType = FRTSAgentFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSAgentInitializer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
	{
		const TArrayView<FRTSAgentFragment> RTSMoveFragmentList = Context.GetMutableFragmentView<FRTSAgentFragment>();
		const FRTSAgentParameters& RTSAgentParameters = Context.GetConstSharedFragment<FRTSAgentParameters>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			UE_LOG(LogTemp, Error, TEXT("INIT ENTITY"));
			// Simply refresh the required resources
			FRTSAgentFragment& RTSAgent = RTSMoveFragmentList[EntityIndex];
			RTSAgent.RequiredResources.Append(RTSAgentParameters.DefaultRequiredResources);
			
			Context.Defer().RemoveTag<FRTSRequestResources>(Context.GetEntity(EntityIndex));
		}
	}));
}

void URTSAgentInitializer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRTSAgentParameters>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FRTSAgent>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FRTSRequestResources>(EMassFragmentPresence::All);
}

void URTSAgentInitializer::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	RTSMovementSubsystem = UWorld::GetSubsystem<URTSMovementSubsystem>(Owner.GetWorld());
	SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(Owner.GetWorld());
}

void URTSAgentInitializer::Register()
{
	Super::Register();
	ObservedType = FRTSRequestResources::StaticStruct();
	UMassObserverRegistry::GetMutable().RegisterObserver(*ObservedType, Operation, GetClass());
}

URTSConstructBuilding::URTSConstructBuilding()
{
	ObservedType = FRTSBuildingFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSConstructBuilding::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FRTSBuildingFragment> BuildingFragments = Context.GetFragmentView<FRTSBuildingFragment>();
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSBuildingFragment& BuildingFragment = BuildingFragments[EntityIndex];
			
			if (const USmartObjectComponent* SmartObjectComponent = SmartObjectSubsystem->GetSmartObjectComponent(BuildingFragment.BuildingClaimHandle))
			{
				const AActor* Actor = SmartObjectComponent->GetOwner();
				UInstancedStaticMeshComponent* InstancedStaticMeshComp = Actor->FindComponentByClass<UInstancedStaticMeshComponent>();
				FTransform Transform;
				Transform.SetLocation(FVector(0,0,IncrementHeight*InstancedStaticMeshComp->GetInstanceCount()));
				InstancedStaticMeshComp->AddInstance(Transform);

				Context.Defer().RemoveFragment<FRTSBuildingFragment>(Context.GetEntity(EntityIndex));
			}
		}
	});
}

void URTSConstructBuilding::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSBuildingFragment>(EMassFragmentAccess::ReadOnly);
}

void URTSConstructBuilding::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(Owner.GetWorld());
}
