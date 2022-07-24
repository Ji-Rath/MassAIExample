// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentTrait.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassRepresentationFragments.h"
#include "SmartObjectSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MassAITesting/RTSBuildingSubsystem.h"

//----------------------------------------------------------------------//
// URTSGatherResourceProcessor
//----------------------------------------------------------------------//
void URTSGatherResourceProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FRTSGatherResourceFragment> GatherResourceFragments = Context.GetFragmentView<FRTSGatherResourceFragment>();
		const TArrayView<FRTSAgentFragment> RTSAgentFragment = Context.GetMutableFragmentView<FRTSAgentFragment>();

		URTSBuildingSubsystem* BuildingSubsystem = GetWorld()->GetSubsystem<URTSBuildingSubsystem>();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FRTSGatherResourceFragment& ResourceFragment = GatherResourceFragments[EntityIndex];
			FRTSAgentFragment& RTSAgent = RTSAgentFragment[EntityIndex];

			// Add the given resource to the agent
			int* InventoryItem = &RTSAgent.Inventory.FindOrAdd(ResourceFragment.Resource);
			*InventoryItem += ResourceFragment.Amount;

			// Subtract from required resources (janky)
			//RTSAgent.QueuedItems.Pop();

			// Remove fragment so we dont infinitely grant resources
			// TODO: Consider using tags rather than just removing the fragment now
			Context.Defer().RemoveFragment<FRTSGatherResourceFragment>(Context.GetEntity(EntityIndex));

			BuildingSubsystem->OnEntityUpdateInventory.Broadcast(Context.GetEntity(EntityIndex));
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
	BuildContext.AddFragment<FAgentAnimationData>();
	BuildContext.AddTag<FRTSAgent>();
	
	const FConstSharedStruct RTSAgentFragment = EntitySubsystem->GetOrCreateConstSharedFragment(UE::StructUtils::GetStructCrc32(FConstStructView::Make(AgentParameters)), AgentParameters);
	BuildContext.AddConstSharedFragment(RTSAgentFragment);
}

//----------------------------------------------------------------------//
// URTSAgentInitializer
//----------------------------------------------------------------------//
URTSAgentInitializer::URTSAgentInitializer()
{
	ObservedType = FRTSAgentFragment::StaticStruct();
	Operation = EMassObservedOperation::Add;
}

void URTSAgentInitializer::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
	{
		const TArrayView<FRTSAgentFragment> RTSMoveFragmentList = Context.GetMutableFragmentView<FRTSAgentFragment>();
		const FRTSAgentParameters& RTSAgentParameters = Context.GetConstSharedFragment<FRTSAgentParameters>();
		TArrayView<FMassRepresentationFragment> RepresentationFragments = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		UMassRepresentationSubsystem* RepresentationSubsystem = GetWorld()->GetSubsystem<UMassRepresentationSubsystem>();
		URTSBuildingSubsystem* BuildingSubsystem = GetWorld()->GetSubsystem<URTSBuildingSubsystem>();
		 
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			// Simply refresh the required resources
			FRTSAgentFragment& RTSAgent = RTSMoveFragmentList[EntityIndex];
			FMassRepresentationFragment& RepresentationFragment = RepresentationFragments[EntityIndex];
			BuildingSubsystem->AddRTSAgent(Context.GetEntity(EntityIndex));
		}
	}));
}

void URTSAgentInitializer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRTSAgentParameters>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FRTSAgent>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
}

void URTSAgentInitializer::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);

	RTSMovementSubsystem = UWorld::GetSubsystem<URTSBuildingSubsystem>(Owner.GetWorld());
	SmartObjectSubsystem = UWorld::GetSubsystem<USmartObjectSubsystem>(Owner.GetWorld());
}

//----------------------------------------------------------------------//
// URTSAnimationProcessor
//----------------------------------------------------------------------//

URTSAnimationProcessor::URTSAnimationProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
}

void URTSAnimationProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
	
	RepresentationSubsystem = UWorld::GetSubsystem<UMassRepresentationSubsystem>(Owner.GetWorld());
}

void URTSAnimationProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FRTSAgent>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentAnimationData>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
}

void URTSAnimationProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(EntitySubsystem, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FMassMoveTargetFragment> MoveFragments = Context.GetFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FMassVelocityFragment> VelocityFragments = Context.GetFragmentView<FMassVelocityFragment>();
		TArrayView<FMassRepresentationFragment> RepresentationFragments = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		TConstArrayView<FMassRepresentationLODFragment> RepresentationLODFragments = Context.GetFragmentView<FMassRepresentationLODFragment>();
		TArrayView<FRTSAgentFragment> AgentFragments = Context.GetMutableFragmentView<FRTSAgentFragment>();
		TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FAgentAnimationData> AgentAnimationDatas = Context.GetMutableFragmentView<FAgentAnimationData>();
		
		FMassInstancedStaticMeshInfoArrayView MeshInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FMassMoveTargetFragment& MoveFragment = MoveFragments[EntityIndex];
			const FMassVelocityFragment& Velocity = VelocityFragments[EntityIndex];
			FMassRepresentationFragment& Representation = RepresentationFragments[EntityIndex];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODFragments[EntityIndex];
			FRTSAgentFragment& AgentFragment = AgentFragments[EntityIndex];
			FAgentAnimationData& AnimationData = AgentAnimationDatas[EntityIndex];
			FTransform& Transform = Transforms[EntityIndex].GetMutableTransform();

			if (AgentFragment.SkinIndex == -1)
				AgentFragment.SkinIndex = FMath::RandRange(0.f,3.f);
			
			// todo, find a way to iterate and update animations based on current action state. can also be used to update other ISM things
			if (Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				float Anim = 0.f;
				const float Speed = Velocity.Value.Length();
				if (Speed > 20.f && Speed < 200.f)
					Anim = 0.5f;
				else if (Speed >= 100.f)
					Anim = 1.f;
				
				AnimationData.AnimState = Anim;
				AnimationData.IsPunching = AgentFragment.bPunching;
				AnimationData.SkinIndex = AgentFragment.SkinIndex;

				MeshInfo[Representation.StaticMeshDescIndex].AddBatchedCustomData(AnimationData, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
			}
		}
	});
}
