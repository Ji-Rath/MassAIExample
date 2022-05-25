// Fill out your copyright notice in the Description page of Project Settings.


#include "NavMeshMovementTrait.h"

#include "AdvancedRandomMovementTrait.h"
#include "AITypes.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "NavigationSystem.h"


struct FMassMoveTargetFragment;
struct FMassMovementParameters;

void UNavMeshMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, UWorld& World) const
{
	BuildContext.AddTag<FNavAgent>();
	BuildContext.AddFragment<FNavMeshPathFragment>();
}

UNavMeshMovementProcessor::UNavMeshMovementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::All;
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UNavMeshMovementProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, ([this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> TransformsList = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FMassMoveTargetFragment> NavTargetsList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();
		const TArrayView<FNavMeshPathFragment> PathList = Context.GetMutableFragmentView<FNavMeshPathFragment>();
		const FMassMovementParameters& MovementParameters = Context.GetConstSharedFragment<FMassMovementParameters>();

		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		
		if (NavSys)
		{
			for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
			{
				// Get fragment data
				const FTransform& Transform = TransformsList[EntityIndex].GetTransform();
				FMassMoveTargetFragment& MoveTarget = NavTargetsList[EntityIndex];
				FNavMeshPathFragment& Path = PathList[EntityIndex];
				int8& PathIndex = Path.PathIndex;

				// Get info from fragments
				FVector CurrentLocation = Transform.GetLocation();
				FVector& TargetVector = Path.TargetLocation;

				// Calculate path
				if (Path.PathResult.Result != ENavigationQueryResult::Success)
				{
					FNavLocation Result;
					NavSys->GetRandomReachablePointInRadius(Transform.GetLocation(), 3000.f, Result);
					Path.TargetLocation = Result.Location;
					
					FAIMoveRequest MoveRequest(TargetVector);
					const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance();
					FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, this, MoveRequest.GetNavigationFilter());
					FPathFindingQuery Query(&*this, *NavData, CurrentLocation, TargetVector, NavFilter);
					Path.PathResult = NavSys->FindPathSync(Query);
					if (Path.PathResult.Result == ENavigationQueryResult::Success)
					{
						PathIndex = 0;
						MoveTarget.Center = Path.PathResult.Path->GetPathPointLocation(1).Position;

						/*
						for(const FNavPathPoint& Point : Path.PathResult.Path->GetPathPoints())
						{
							UE_LOG(LogTemp, Warning, TEXT("Points: %s"), *(Point.Location.ToString()));
						}
						*/
					}

					// Use result to move ai

					//DrawDebugLine(GetWorld(), Transform.GetLocation(),MoveTarget.Center, FColor::Red, true, -1, 0, 5);
					MoveTarget.DesiredSpeed = FMassInt16Real(MovementParameters.DefaultDesiredSpeed);
				}
				MoveTarget.DistanceToGoal = (MoveTarget.Center-Transform.GetLocation()).Size();
				MoveTarget.Forward = (MoveTarget.Center-Transform.GetLocation()).GetSafeNormal();

				if (MoveTarget.DistanceToGoal <= 50.f)
				{
					if (PathIndex < Path.PathResult.Path->GetPathPoints().Num()-1)
					{
						PathIndex++;
						MoveTarget.Center = Path.PathResult.Path->GetPathPoints()[PathIndex];
					}
					else
					{
						PathIndex = 0;
						Path.PathResult.Result = ENavigationQueryResult::Invalid;
						FNavLocation Result;
						NavSys->GetRandomReachablePointInRadius(Transform.GetLocation(), 3000.f, Result);
						Path.TargetLocation = Result.Location;
						UE_LOG(LogTemp, Error, TEXT("Current Destination: %s"), *(Path.TargetLocation.ToString()));
					}
				}
				
				//UE_LOG(LogTemp, Error, TEXT("Current Destination: %s"), *(MoveTarget.Center.ToString()));
				//UE_LOG(LogTemp, Error, TEXT("Distance to goal: %f"), MoveTarget.DistanceToGoal);
				//UE_LOG(LogTemp, Error, TEXT("Forward: %s"), *(MoveTarget.Forward.ToString()));
				//UE_LOG(LogTemp, Error, TEXT("Current Location: %s"), *(Transform.GetLocation().ToString()));

				//DrawDebugLine(GetWorld(), Transform.GetLocation(), Transform.GetLocation()+(MoveTarget.Forward*200), FColor::Green, false, 0.1, 5);
				// TODO: Track path index and goal to determine when finished and if it may change
			}
		}
	}));
}

void UNavMeshMovementProcessor::ConfigureQueries()
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FNavMeshPathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddConstSharedRequirement<FMassMovementParameters>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FNavAgent>(EMassFragmentPresence::All);
}
