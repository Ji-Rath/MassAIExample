// Fill out your copyright notice in the Description page of Project Settings.


#include "Mass/Animation/VertexAnimProcessor.h"

#include "AnimToTextureInstancePlaybackHelpers.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationProcessor.h"
#include "MassRepresentationSubsystem.h"
#include "MassSimulationLOD.h"

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
	
	auto ShouldRun = [](const FMassExecutionContext& Context)
	{
		return FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk(Context);
	};
	EntityQuery.SetChunkFilter(ShouldRun);
	
	EntityQuery.RegisterWithProcessor(*this);
}

void UVertexAnimProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
	{
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