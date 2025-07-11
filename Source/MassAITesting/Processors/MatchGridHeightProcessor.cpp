// Fill out your copyright notice in the Description page of Project Settings.


#include "MatchGridHeightProcessor.h"

#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassSimulationLOD.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "MassAITesting/Fragments/GridFragments.h"
#include "MassAITesting/Subsystems/GridManagerSubsystem.h"

UMatchGridHeightProcessor::UMatchGridHeightProcessor() :
	EntityQuery(*this)
{
}

void UMatchGridHeightProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FGridFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UGridManagerSubsystem>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);

	// Optimization
	EntityQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
	EntityQuery.AddTagRequirement<FMassVisibilityCanBeSeenTag>(EMassFragmentPresence::All);
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.SetChunkFilter([](const FMassExecutionContext& MassContext)
	{
		return FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk(MassContext) && FMassSimulationVariableTickChunkFragment::ShouldTickChunkThisFrame(MassContext);
	});
}

void UMatchGridHeightProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		auto& GridManagerSubsystem = Context.GetSubsystemChecked<UGridManagerSubsystem>();
		
		const auto TransformFragments = Context.GetMutableFragmentView<FTransformFragment>();
		const auto GridFragments = Context.GetMutableFragmentView<FGridFragment>();
		const auto MoveFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			auto& TransformFragment = TransformFragments[EntityIdx];
			auto& GridFragment = GridFragments[EntityIdx];
			auto& MoveFragment = MoveFragments[EntityIdx];
			auto Location = TransformFragment.GetTransform().GetLocation();

			// Only update nearby nodes once we are a certain distance away
			float DistToLastPos = FVector::Dist2D(GridFragment.LastQueriedPosition, Location);
			if (DistToLastPos > 500.f)
			{
				SCOPED_NAMED_EVENT(STAT_GetNearbyNodes, FColor::Red);
				GridManagerSubsystem.GetNearbyNodes(Location, GridFragment.NearbyNodes);
				GridFragment.LastQueriedPosition = Location;
			}

			FVector ClosestLocation = FVector::ZeroVector;
			float ClosestDistance = FLT_MAX;
			{
				SCOPED_NAMED_EVENT(STAT_CalculateNodeDistance, FColor::Red);
				
				for (int32 NearbyNode : GridFragment.NearbyNodes)
				{
					FTransform Transform;
					GridManagerSubsystem.GridMesh->GetInstanceTransform(NearbyNode, Transform, true);
					float Dist = FVector::DistSquared2D(Transform.GetLocation(), Location);
					
					if (Dist < ClosestDistance)
					{
						ClosestLocation = Transform.GetLocation();
						ClosestDistance = Dist;
					}
				}
			}
			//DrawDebugPoint(Context.GetWorld(), ClosestLocation, 100.f, FColor::Yellow, false);
			// Align entity to grid
			ClosestLocation.X = Location.X;
			ClosestLocation.Y = Location.Y;
			ClosestLocation.Z += 90.f;
			//TransformFragment.GetMutableTransform().SetLocation(ClosestLocation);
			MoveFragment.Center.Z = ClosestLocation.Z;
		}
	});
}
