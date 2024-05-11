// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSAgentTrait.h"

#include "AnimToTextureDataAsset.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassRepresentationFragments.h"
#include "SmartObjectSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "AnimToTextureInstancePlaybackHelpers.h"
#include "MassEntitySubsystem.h"
#include "MassAITesting/RTSBuildingSubsystem.h"

//----------------------------------------------------------------------//
// URTSGatherResourceProcessor
//----------------------------------------------------------------------//
void URTSGatherResourceProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, ([this](FMassExecutionContext& Context)
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
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSGatherResourceProcessor::Initialize(UObject& Owner)
{
	Super::Initialize(Owner);
}

//----------------------------------------------------------------------//
// URTSAgentTrait
//----------------------------------------------------------------------//
void URTSAgentTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	UMassEntitySubsystem* EntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(&World);
	check(EntitySubsystem);
	
	BuildContext.AddFragment<FRTSAgentFragment>();
	BuildContext.AddFragment<FRTSAnimationFragment>();
	BuildContext.AddTag<FRTSAgent>();
	
	const FConstSharedStruct RTSAgentFragment = EntitySubsystem->GetMutableEntityManager().GetOrCreateConstSharedFragment(AgentParameters);
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

void URTSAgentInitializer::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, ([this](FMassExecutionContext& Context)
	{
		const TArrayView<FRTSAgentFragment> RTSMoveFragmentList = Context.GetMutableFragmentView<FRTSAgentFragment>();
		const FRTSAgentParameters& RTSAgentParameters = Context.GetConstSharedFragment<FRTSAgentParameters>();
		TArrayView<FRTSAnimationFragment> AnimationFragments = Context.GetMutableFragmentView<FRTSAnimationFragment>();
		TArrayView<FMassRepresentationFragment> RepresentationFragments = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		UMassRepresentationSubsystem* RepresentationSubsystem = GetWorld()->GetSubsystem<UMassRepresentationSubsystem>();
		URTSBuildingSubsystem* BuildingSubsystem = GetWorld()->GetSubsystem<URTSBuildingSubsystem>();
		 
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			// Simply refresh the required resources
			FRTSAgentFragment& RTSAgent = RTSMoveFragmentList[EntityIndex];
			FMassRepresentationFragment& RepresentationFragment = RepresentationFragments[EntityIndex];
			FRTSAnimationFragment& AnimationFragment = AnimationFragments[EntityIndex];
			BuildingSubsystem->AddRTSAgent(Context.GetEntity(EntityIndex));

			UAnimToTextureDataAsset* Anim = RTSAgentParameters.AnimData.Get();
			if (!Anim)
				Anim = RTSAgentParameters.AnimData.LoadSynchronous();
			AnimationFragment.AnimToTextureData = Anim;
		}
	}));
}

void URTSAgentInitializer::ConfigureQueries()
{
	EntityQuery.AddRequirement<FRTSAgentFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRTSAgentParameters>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FRTSAgent>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FRTSAnimationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.RegisterWithProcessor(*this);
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
	EntityQuery.AddRequirement<FRTSAnimationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FRTSAgentParameters>(EMassFragmentPresence::All);
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	EntityQuery.RegisterWithProcessor(*this);
}

void URTSAnimationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TConstArrayView<FMassMoveTargetFragment> MoveFragments = Context.GetFragmentView<FMassMoveTargetFragment>();
		TConstArrayView<FMassVelocityFragment> VelocityFragments = Context.GetFragmentView<FMassVelocityFragment>();
		TArrayView<FMassRepresentationFragment> RepresentationFragments = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		TConstArrayView<FMassRepresentationLODFragment> RepresentationLODFragments = Context.GetFragmentView<FMassRepresentationLODFragment>();
		TArrayView<FRTSAgentFragment> AgentFragments = Context.GetMutableFragmentView<FRTSAgentFragment>();
		TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		TArrayView<FRTSAnimationFragment> AgentAnimationDatas = Context.GetMutableFragmentView<FRTSAnimationFragment>();

		const FRTSAgentParameters& AgentParameters = Context.GetConstSharedFragment<FRTSAgentParameters>();
		
		FMassInstancedStaticMeshInfoArrayView MeshInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		
		for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
		{
			const FMassMoveTargetFragment& MoveFragment = MoveFragments[EntityIndex];
			const FMassVelocityFragment& Velocity = VelocityFragments[EntityIndex];
			FMassRepresentationFragment& Representation = RepresentationFragments[EntityIndex];
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODFragments[EntityIndex];
			FRTSAgentFragment& AgentFragment = AgentFragments[EntityIndex];
			FRTSAnimationFragment& AnimationData = AgentAnimationDatas[EntityIndex];
			FTransform& Transform = Transforms[EntityIndex].GetMutableTransform();

			if (AgentFragment.SkinIndex == -1)
				AgentFragment.SkinIndex = FMath::RandRange(0.f,3.f);
			
			// todo, find a way to iterate and update animations based on current action state. can also be used to update other ISM things
			if (Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				// 0-4 is anim data

				const float PrevPlayRate = AnimationData.PlayRate;
				float GlobalTime = GetWorld()->GetTimeSeconds();
				if (AnimationData.bCustomAnimation)
				{
					// Custom animation - handled by other processor/task
					AnimationData.PlayRate = 2.f;

					// Need to conserve current frame on a playrate switch so (GlobalTime - Offset1) * Playrate1 == (GlobalTime - Offset2) * Playrate2
					//AnimationData.GlobalStartTime = (GlobalTime - AnimationData.GlobalStartTime) / AnimationData.PlayRate;// - AnimationData.AnimPosition;
					AnimationData.GlobalStartTime = GlobalTime - PrevPlayRate * (GlobalTime - AnimationData.GlobalStartTime) / AnimationData.PlayRate;
					//AnimationData.AnimPosition += 1;
				}
				else
				{
					int32 Anim = 0;
					const float SpeedSq = Velocity.Value.SizeSquared();
					const float StandCutoff = 20.f;
					const float WalkCutoff = 300.f;
					if (SpeedSq > StandCutoff*StandCutoff && SpeedSq <= WalkCutoff*WalkCutoff)
					{
						Anim = 1;
						AnimationData.PlayRate = FMath::Clamp(FMath::Sqrt(SpeedSq), 0.6f, 2.0f);
					}
					if (SpeedSq > WalkCutoff*WalkCutoff)
					{
						Anim = 2;
						AnimationData.PlayRate = FMath::Clamp(FMath::Sqrt(SpeedSq / (WalkCutoff*WalkCutoff)), 0.6f, 2.0f);
					}
					Anim = AgentFragment.bPunching ? 3 : Anim;
					AnimationData.AnimationStateIndex = Anim;
					
					// Need to conserve current frame on a playrate switch so (GlobalTime - Offset1) * Playrate1 == (GlobalTime - Offset2) * Playrate2
					AnimationData.GlobalStartTime = GlobalTime - PrevPlayRate * (GlobalTime - AnimationData.GlobalStartTime) / AnimationData.PlayRate;
					//UE_LOG(LogTemp, Error, TEXT("Global Start Time: %f"), AnimationData.GlobalStartTime);
				}
				
				
				//AnimationData.IsPunching = AgentFragment.bPunching;
				//AnimationData.SkinIndex = AgentFragment.SkinIndex;

				//@todo update skinindex and punching custom data (might be useful to use montage method in City Sample)
				
				UpdateISMVertexAnimation(MeshInfo[Representation.StaticMeshDescIndex], AnimationData, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance, 0);
				MeshInfo[Representation.StaticMeshDescIndex].AddBatchedCustomData<float>(AgentFragment.SkinIndex, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance, 4);
			}
		}
	});
}

void URTSAnimationProcessor::UpdateISMVertexAnimation(FMassInstancedStaticMeshInfo& ISMInfo, FRTSAnimationFragment& AnimationData, const float LODSignificance, const float PrevLODSignificance, const int32 NumFloatsToPad /*= 0*/)
{
	/*
	FAnimToTextureInstancePlaybackData InstanceData;
	UAnimToTextureInstancePlaybackLibrary::GetFrameDataFromDataAsset(AnimationData.AnimToTextureData.Get(), AnimationData.AnimationStateIndex, InstanceData.CurrentState);
	InstanceData.CurrentState.GlobalStartTime = AnimationData.GlobalStartTime;
	InstanceData.CurrentState.PlayRate = AnimationData.PlayRate;
	ISMInfo.AddBatchedCustomData<FAnimToTextureInstancePlaybackData>(InstanceData, LODSignificance, PrevLODSignificance, NumFloatsToPad);
	*/
}
