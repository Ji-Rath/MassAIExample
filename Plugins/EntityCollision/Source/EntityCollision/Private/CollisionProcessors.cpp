// Fill out your copyright notice in the Description page of Project Settings.


#include "CollisionProcessors.h"

#include "CollisionFragments.h"
#include "CollisionSubsystem.h"
#include "LineTypes.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "MassLODFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationDebug.h"
#include "MassSimulationLOD.h"
#include "Algo/RandomShuffle.h"
#include "Movement/MassMovementProcessors.h"

UCollisionInitializerProcessor::UCollisionInitializerProcessor() :
	EntityQuery(*this)
{
	ObservedType = FCollisionFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Add;
}

void UCollisionInitializerProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FObstacleTag>(EMassFragmentPresence::All);
}

void UCollisionInitializerProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		UCollisionSubsystem& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();

		TArrayView<FCollisionFragment> HashGridFragments = Context.GetMutableFragmentView<FCollisionFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			FCollisionFragment& HashGridFragment = HashGridFragments[EntityIdx];
			const FTransformFragment& TransformFragment = TransformFragments[EntityIdx];
			const FVector& Location = TransformFragment.GetTransform().GetLocation();
			float HalfRadius = RadiusFragments[EntityIdx].Radius / 2;
			
			FBox Bounds = { Location - HalfRadius, Location + HalfRadius };
			HashGridFragment.CellLocation = HashGridSubsystem.HashGridData.Add(Context.GetEntity(EntityIdx), Bounds);
		}
	});
}

UCollisionDestroyProcessor::UCollisionDestroyProcessor() :
	EntityQuery(*this)
{
	ObservedType = FCollisionFragment::StaticStruct();
	ObservedOperations = EMassObservedOperationFlags::Remove;
}

void UCollisionDestroyProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);
}

void UCollisionDestroyProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		UCollisionSubsystem& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();
		
		TConstArrayView<FCollisionFragment> HashGridFragments = Context.GetFragmentView<FCollisionFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			const FCollisionFragment& HashGridFragment = HashGridFragments[EntityIdx];

			HashGridSubsystem.HashGridData.Remove(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation);
		}
	});
}

UCollisionProcessor::UCollisionProcessor() :
	EntityQuery(*this),
	AvoidanceQuery(*this)
{
	ExecutionFlags = int32(EProcessorExecutionFlags::AllNetModes);
	ExecutionOrder.ExecuteInGroup = (UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCollisionProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FCollisionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddTagRequirement<FObstacleTag>(EMassFragmentPresence::All);
	
	AvoidanceQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	AvoidanceQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
	AvoidanceQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	AvoidanceQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	AvoidanceQuery.AddSubsystemRequirement<UCollisionSubsystem>(EMassFragmentAccess::ReadOnly);
	AvoidanceQuery.AddConstSharedRequirement<FAvoidanceSettings>();
	AvoidanceQuery.AddTagRequirement<FEntityAvoidanceTag>(EMassFragmentPresence::All);
	AvoidanceQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
	AvoidanceQuery.AddTagRequirement<FMassLowLODTag>(EMassFragmentPresence::Optional);
	
#if WITH_MASSGAMEPLAY_DEBUG
	AvoidanceQuery.DebugEnableEntityOwnerLogging();
#endif
}

void UCollisionProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// Update hash grid position
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(UpdateCollisionHashGrid)
		UCollisionSubsystem& HashGridSubsystem = Context.GetMutableSubsystemChecked<UCollisionSubsystem>();

		TArrayView<FCollisionFragment> HashGridFragments = Context.GetMutableFragmentView<FCollisionFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		
		const int32 NumEntities = Context.GetNumEntities();
		for (int EntityIdx = 0; EntityIdx < NumEntities; EntityIdx++)
		{
			FCollisionFragment& HashGridFragment = HashGridFragments[EntityIdx];
			const FTransformFragment& TransformFragment = TransformFragments[EntityIdx];
			const FVector& Location = TransformFragment.GetTransform().GetLocation();
			const float HalfRadius = RadiusFragments[EntityIdx].Radius / 2;

			// Move the entity to the new location
			FBox Bounds = { Location - HalfRadius, Location + HalfRadius };
			FHashGridExample::FCellLocation NewCellLocation = HashGridSubsystem.HashGridData.Move(Context.GetEntity(EntityIdx), HashGridFragment.CellLocation, Bounds);
			HashGridFragment.CellLocation = NewCellLocation;
		}
	});

	// Use ORCA for nearby entity avoidance
	// This also includes collision handling
	AvoidanceQuery.ParallelForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(AvoidanceQuery)
		TArrayView<FMassDesiredMovementFragment> DesiredMovementFragments = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
		TArrayView<FMassForceFragment> ForceFragments = Context.GetMutableFragmentView<FMassForceFragment>();
		
		const FAvoidanceSettings& AvoidanceSettings = Context.GetConstSharedFragment<FAvoidanceSettings>();
		const UCollisionSubsystem& CollisionSubsystem = Context.GetSubsystemChecked<UCollisionSubsystem>();
		TConstArrayView<FAgentRadiusFragment> RadiusFragments = Context.GetFragmentView<FAgentRadiusFragment>();
		TConstArrayView<FTransformFragment> TransformFragments = Context.GetFragmentView<FTransformFragment>();
		
		bool bCalculateAvoidance = !Context.DoesArchetypeHaveTag<FMassLowLODTag>() && !Context.DoesArchetypeHaveTag<FMassMediumLODTag>();
		
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FMassForceFragment& ForceFragment = ForceFragments[EntityIt];
			FVector& DesiredVelocity = DesiredMovementFragments[EntityIt].DesiredVelocity;
			const FAgentRadiusFragment& RadiusFragment = RadiusFragments[EntityIt];
			const FTransformFragment& TransformFragment = TransformFragments[EntityIt];
			
			const FVector& EntityLocation = TransformFragment.GetTransform().GetLocation();
			
			TArray<FMassEntityHandle, TInlineAllocator<MaxObstacleResults>> NearbyObstacles;
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(QueryCloseObstacles)
				FindCloseObstacles<FHashGridExample, FHashGridExample::FCell, FHashGridExample::FItem, FMassEntityHandle, MaxObstacleResults>(EntityLocation, AvoidanceSettings.ObstacleSearchDistance, CollisionSubsystem.HashGridData, NearbyObstacles);
			}
			
			// No avoidance logic to run
			if (NearbyObstacles.Num() == 0) { continue; }
			
			TArray<UE::Geometry::FLine2f> OrcaLines;
			bool bColliding = false;
			
			auto ORCAAvoidance = [&](const FVector& OtherLocation, const float& OtherRadius, const FVector& OtherVelocity)
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(CalculateORCALines)
				const float Radius = RadiusFragment.Radius + 5.f;
				
				FVector2f EntityLocation2D(EntityLocation.X, EntityLocation.Y);
				FVector2f EntityVelocity2D(DesiredVelocity.X, DesiredVelocity.Y);
				
				FVector2f OtherLocation2D(OtherLocation.X, OtherLocation.Y);
				FVector2f OtherVelocity2D(OtherVelocity.X, OtherVelocity.Y);
				
				FVector2f RelativePosition = OtherLocation2D - EntityLocation2D;
				FVector2f RelativeVelocity =  EntityVelocity2D - OtherVelocity2D;
				
				float DistSq = FVector::DistSquared(EntityLocation, OtherLocation);
				float CombinedRadius = Radius + OtherRadius;
				float CombinedRadiusSq = CombinedRadius * CombinedRadius;
				
				const float InvTimeHorizon = 1.f / AvoidanceSettings.TimeHorizon;
				
				UE::Geometry::FLine2f Line;
				FVector2f U;
				
				// No collision
				if (DistSq > CombinedRadiusSq)
				{
					FVector2f W = RelativeVelocity - InvTimeHorizon * RelativePosition;
					const float WLengthSq = W.SizeSquared();
					
					const float Dot = FVector2f::DotProduct(W, RelativePosition);
					
					if (Dot < 0.f && Dot * Dot > CombinedRadiusSq * WLengthSq)
					{
						// Project on cut-off circle
						const float WLength = FMath::Sqrt(WLengthSq);
						const FVector2f UnitW = W / WLength;
						
						Line.Direction = FVector2f(UnitW.Y, -UnitW.X);
						U = (CombinedRadius * InvTimeHorizon - WLength) * UnitW;
					}
					else
					{
						// Project on legs
						const float Leg = FMath::Sqrt(DistSq - CombinedRadiusSq);
						
						if (FVector2f::CrossProduct(RelativePosition, W) > 0.f)
						{
							// Project on left leg
							Line.Direction = FVector2f(RelativePosition.X * Leg - RelativePosition.Y * CombinedRadius,
								RelativePosition.X * CombinedRadius + RelativePosition.Y * Leg) / DistSq;
						}
						else
						{
							// Project on right leg
							Line.Direction = -FVector2f(RelativePosition.X * Leg + RelativePosition.Y * CombinedRadius,
								-RelativePosition.X * CombinedRadius + RelativePosition.Y * Leg) / DistSq;
						}
						
						U = FVector2f::DotProduct(RelativeVelocity, Line.Direction) * Line.Direction - RelativeVelocity;
					}
				}
				else
				{
					// Collision
					const float InvTimeStep = 1.f / Context.GetDeltaTimeSeconds();
					
					const FVector2f W = RelativeVelocity - InvTimeStep * RelativePosition;
					const float WLength = W.Size();
					const FVector2f UnitW = W / WLength;
					
					Line.Direction = FVector2f(UnitW.Y, -UnitW.X);
					U = (CombinedRadius * InvTimeStep - WLength) * UnitW;
				}
				
				Line.Origin = EntityVelocity2D + FVector2f(0.5f) * U;
				OrcaLines.Emplace(Line);
				
		#if WITH_MASSGAMEPLAY_DEBUG
				const UE::MassNavigation::Debug::FDebugContext CollisionDebugContext(Context, this, LogTemp, Context.GetWorld(), Context.GetEntity(EntityIt), EntityIt);
				UE::MassNavigation::Debug::DebugDrawLine(CollisionDebugContext, OtherLocation + OtherVelocity + FVector(Line.Origin.X, Line.Origin.Y, 0.f), OtherLocation + OtherVelocity + FVector(Line.Origin.X, Line.Origin.Y, 0.f) + FVector(Line.Direction.X, Line.Direction.Y, 0.f) * 500.f, FColor::Green, 5.f);
		#endif
			};
			
			auto CollisionAndSliding = [&](const FVector& OtherLocation, const float& OtherRadius)
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(SoftCollision)
				const float Radius = RadiusFragment.Radius;
				
				float MinDistance = Radius + OtherRadius;
			
				FVector ToNeighbor = OtherLocation - EntityLocation;
				float Distance = ToNeighbor.Size();
			
				if (Distance < MinDistance + 5 && Distance > UE_KINDA_SMALL_NUMBER)
				{
					bColliding = true;
					float Overlap = MinDistance - Distance;
					FVector PushDir = ToNeighbor / Distance * -1.0f;
				
					FVector ToNeighborNormal = ToNeighbor.GetSafeNormal();
					float VelDot = FVector::DotProduct(DesiredVelocity, ToNeighborNormal);
					
					if (Distance < MinDistance)
					{
						DesiredVelocity += PushDir * (Overlap / MinDistance) * AvoidanceSettings.PushStrength;
					}
					else
					{
						if (VelDot > 0.f)
						{
							DesiredVelocity = DesiredVelocity - (ToNeighborNormal * VelDot);
							ForceFragment.Value = FVector::ZeroVector;
						}
					}
				}
			};
			
			for (const FMassEntityHandle& NearbyObstacle : NearbyObstacles)
			{
				FMassEntityView EntityView(Context.GetEntityManagerChecked(), NearbyObstacle);
				const FVector& OtherLocation = EntityView.GetFragmentData<FTransformFragment>().GetTransform().GetLocation();
				const float OtherRadius = EntityView.GetFragmentData<FAgentRadiusFragment>().Radius;

				FMassDesiredMovementFragment* DesiredMovementFragment = EntityView.GetFragmentDataPtr<FMassDesiredMovementFragment>();
				const FVector& OtherVelocity = DesiredMovementFragment ? DesiredMovementFragment->DesiredVelocity : FVector::ZeroVector;
				
				if (bCalculateAvoidance)
				{
					ORCAAvoidance(OtherLocation, OtherRadius, OtherVelocity);
				}
				
				CollisionAndSliding(OtherLocation, OtherRadius);
			}
			
			// Avoidance
			if (!bColliding && bCalculateAvoidance)
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(Avoidance)
			
				FVector2f NewVelocity = ResolveCollisions(OrcaLines, Context, EntityLocation, DesiredVelocity, EntityIt);
				FVector NewVelocity3D = FVector(NewVelocity.X, NewVelocity.Y, 0.f);
				
				DesiredVelocity = FMath::VInterpTo(DesiredVelocity, NewVelocity3D, Context.GetDeltaTimeSeconds(), 10.f);
			}
			
			DesiredVelocity.Z = 0.f;
			DesiredVelocity = DesiredVelocity.GetClampedToMaxSize(200.f);
		}
	});
}

FVector2f UCollisionProcessor::ResolveCollisions(const TConstArrayView<UE::Geometry::FLine2f>& OrcaLines,
                                                 FMassExecutionContext& Context,
                                                 const FVector& EntityLocation, const FVector& EntityVelocity, const FMassExecutionContext::FEntityIterator& EntityIt)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CalculateCollision)
	FVector2f NewVelocity(FVector2f::ZeroVector);
	FVector2f EntityVelocity2D(EntityVelocity.X, EntityVelocity.Y);
	
	bool bResult = LinearProgram2(OrcaLines, 200.f, EntityVelocity2D, false, NewVelocity);
	
#if WITH_MASSGAMEPLAY_DEBUG
	const UE::MassNavigation::Debug::FDebugContext CollisionDebugContext(Context, this, LogTemp, Context.GetWorld(), Context.GetEntity(EntityIt), EntityIt);
	UE::MassNavigation::Debug::DebugDrawLine(CollisionDebugContext, EntityLocation, EntityLocation + FVector(NewVelocity.X, NewVelocity.Y, 0.f), bResult ? FColor::Red : FColor::Cyan, 15.f, false, NewVelocity.ToString());
#endif
	
	return NewVelocity;
}

bool UCollisionProcessor::LinearProgram2(const TConstArrayView<UE::Geometry::FLine2f>& Lines, float Radius, FVector2f OptVelocity, bool DirectionOpt, FVector2f& OutResult)
{
	if (DirectionOpt) {
		OutResult = OptVelocity * Radius;
	} else if (OptVelocity.SizeSquared() > FMath::Square(Radius)) {
		OutResult = OptVelocity.GetSafeNormal() * Radius;
	} else {
		OutResult = OptVelocity;
	}

	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		// If current result violates the new constraint i
		if (FVector2f::CrossProduct(Lines[i].Direction, Lines[i].Origin - OutResult) > 0.0f)
		{
			const FVector2f TempResult = OutResult;
			
			// linearProgram1: Optimize along the line 'i' subject to previous lines 0 to i-1
			if (!LinearProgram1(Lines, i, Radius, OptVelocity, DirectionOpt, OutResult))
			{
				OutResult = TempResult;
				return false;
			}
		}
	}
	return true;
}

bool UCollisionProcessor::LinearProgram1(const TConstArrayView<UE::Geometry::FLine2f>& Lines, int32 LineNo, float Radius, FVector2f OptVelocity, bool DirectionOpt, FVector2f& OutResult)
{
	// Projection of the origin onto the line
	float DotProduct = Lines[LineNo].Origin | Lines[LineNo].Direction;
	float Discriminant = FMath::Square(DotProduct) + FMath::Square(Radius) - Lines[LineNo].Origin.SizeSquared();

	if (Discriminant < 0.0f) return false; // Max speed too small to satisfy constraint

	float SqrtDiscriminant = FMath::Sqrt(Discriminant);
	float tLeft = -DotProduct - SqrtDiscriminant;
	float tRight = -DotProduct + SqrtDiscriminant;

	for (int32 i = 0; i < LineNo; ++i)
	{
		float Denominator = FVector2f::CrossProduct(Lines[LineNo].Direction, Lines[i].Direction);
		float Numerator = FVector2f::CrossProduct(Lines[i].Direction, Lines[LineNo].Origin - Lines[i].Origin);

		if (FMath::Abs(Denominator) <= KINDA_SMALL_NUMBER)
		{
			// Lines are parallel; check if they are mutually exclusive
			if (Numerator < 0.0f) return false;
			continue;
		}

		float t = Numerator / Denominator;
		if (Denominator > 0.0f) tRight = FMath::Min(tRight, t);
		else tLeft = FMath::Max(tLeft, t);

		if (tLeft > tRight) return false;
	}

	// Optimization step
	if (DirectionOpt) {
		OutResult = Lines[LineNo].Origin + (tRight > 0.0f ? tRight : tLeft) * Lines[LineNo].Direction;
	} else {
		float t = Lines[LineNo].Direction | (OptVelocity - Lines[LineNo].Origin);
		t = FMath::Clamp(t, tLeft, tRight);
		OutResult = Lines[LineNo].Origin + t * Lines[LineNo].Direction;
	}

	return true;
}
