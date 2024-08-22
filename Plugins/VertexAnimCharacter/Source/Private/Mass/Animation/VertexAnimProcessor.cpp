// Fill out your copyright notice in the Description page of Project Settings.


#include "Mass/Animation/VertexAnimProcessor.h"

#include "AnimToTextureInstancePlaybackHelpers.h"
#include "MassActorSubsystem.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationProcessor.h"
#include "MassRepresentationSubsystem.h"
#include "MassSimulationLOD.h"
#include "VertexAnimInstance.h"

UVertexAnimProcessor::UVertexAnimProcessor()
{
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
}

void UVertexAnimProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FVertexAnimInfoFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.SetChunkFilter(FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	EntityQuery.RegisterWithProcessor(*this);

	UpdateAnimInstanceQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	UpdateAnimInstanceQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);
	UpdateAnimInstanceQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
	UpdateAnimInstanceQuery.SetChunkFilter(FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	UpdateAnimInstanceQuery.RegisterWithProcessor(*this);
}

void UVertexAnimProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UpdateVertexAnim)
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetMutableSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		
		const TConstArrayView<FMassRepresentationFragment> RepresentationFragmentList = Context.GetFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODFragmentList = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FVertexAnimInfoFragment> VertexAnimationInfoFragmentList = Context.GetMutableFragmentView<FVertexAnimInfoFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const FMassRepresentationFragment& RepresentationFragment = RepresentationFragmentList[EntityIdx];
			const FMassRepresentationLODFragment& RepresentationLODFragment = RepresentationLODFragmentList[EntityIdx];
			FVertexAnimInfoFragment& VertexAnimInfoFragment = VertexAnimationInfoFragmentList[EntityIdx];
			
			if (RepresentationFragment.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance)
			{
				if (RepresentationFragment.StaticMeshDescHandle.IsValid())
				{
					auto SMDesc = RepresentationFragment.StaticMeshDescHandle.ToIndex();
					auto& ISMInfo = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos()[SMDesc];
				
					const float PrevPlayRate = VertexAnimInfoFragment.PlayRate;
					float GlobalTime = GetWorld()->GetTimeSeconds();
				
					// Need to conserve current frame on a playrate switch so (GlobalTime - Offset1) * Playrate1 == (GlobalTime - Offset2) * Playrate2
					VertexAnimInfoFragment.GlobalStartTime = GlobalTime - PrevPlayRate * (GlobalTime - VertexAnimInfoFragment.GlobalStartTime) / VertexAnimInfoFragment.PlayRate;
				
					UpdateISMVertexAnimation(ISMInfo, VertexAnimInfoFragment, RepresentationLODFragment.LOD, RepresentationLODFragment.PrevLOD, 0);
				}
			}
		}
	});

	UpdateAnimInstanceQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UpdateAnimInstance)
		const auto& VelocityFragments = Context.GetFragmentView<FMassVelocityFragment>();
		const auto& ActorFragments = Context.GetFragmentView<FMassActorFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const auto& VelocityFragment = VelocityFragments[EntityIdx];
			const auto& ActorFragment = ActorFragments[EntityIdx];

			UpdateAnimInstance(VelocityFragment, ActorFragment);
		}
	});
}

void UVertexAnimProcessor::UpdateISMVertexAnimation(FMassInstancedStaticMeshInfo& ISMInfo, FVertexAnimInfoFragment& AnimationData, const float LODSignificance, const float PrevLODSignificance, const int32 NumFloatsToPad /*= 0*/)
{
	FAnimToTextureAutoPlayData PlayData;
	if (UAnimToTextureInstancePlaybackLibrary::GetAutoPlayDataFromDataAsset(AnimationData.AnimToTextureData.LoadSynchronous(), AnimationData.AnimationStateIndex, PlayData))
	{
		if (PrevLODSignificance >= 4.f) { return; }
		ISMInfo.AddBatchedCustomData<FAnimToTextureAutoPlayData>(PlayData, LODSignificance, PrevLODSignificance, NumFloatsToPad);
	}
}

void UVertexAnimProcessor::UpdateAnimInstance(const FMassVelocityFragment& VelocityFragment,
	const FMassActorFragment& ActorFragment)
{
	auto Actor = ActorFragment.Get();
	if (!Actor) { return; }
	
	auto SKComp = Actor->GetComponentByClass<USkeletalMeshComponent>();
	if (!SKComp) { return; }
	
	if (auto ActorAnimInstance = Cast<UVertexAnimInstance>(SKComp->GetAnimInstance()))
	{
		ActorAnimInstance->SetVelocity(VelocityFragment.Value);
	}
}
