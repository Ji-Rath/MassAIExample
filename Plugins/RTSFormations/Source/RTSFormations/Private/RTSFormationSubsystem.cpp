// Fill out your copyright notice in the Description page of Project Settings.


#include "RTSFormationSubsystem.h"

#include "DrawDebugHelpers.h"
#include "MassAgentComponent.h"
#include "MassCommonFragments.h"
#include "MassEntityBuilder.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassNavigationFragments.h"
#include "MassObserverNotificationTypes.h"
#include "MassSignalSubsystem.h"
#include "RTSAgentTraits.h"
#include "RTSSignals.h"
#include "Engine/World.h"
#include "ProfilingDebugging/ScopedTimers.h"
#include "Unit/UnitFragments.h"

TArray<FUnitHandle> URTSFormationSubsystem::GetUnits() const
{
	TArray<FUnitHandle> UnitArray;

	auto& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	EntityManager.ForEachSharedFragment<FUnitFragment>([&UnitArray](FUnitFragment& UnitFragment)
	{
		UnitArray.Emplace(UnitFragment.UnitHandle);
	});

	return UnitArray;
}

FUnitHandle URTSFormationSubsystem::GetFirstUnit() const
{
	return GetUnits().IsEmpty() ? FUnitHandle() : GetUnits()[0];
}

void URTSFormationSubsystem::DestroyEntity(UMassAgentComponent* Entity)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	check(EntitySubsystem);
	
	EntitySubsystem->GetEntityManager().Defer().DestroyEntity(Entity->GetEntityHandle());
}

void URTSFormationSubsystem::UpdateUnitPosition(const FUnitHandle& UnitHandle)
{
	auto& InEntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());
	
	InEntityManager.ForEachSharedFragmentConditional<FUnitFragment>([&UnitHandle](const FUnitFragment& InUnitFragment)
	{
		return InUnitFragment.UnitHandle == UnitHandle;
	},
	[&InEntityManager, &UnitHandle](FUnitFragment& UnitFragment)
	{
		TArray<FVector3f> NewPositions;

		FMassEntityQuery EntityQuery(InEntityManager.AsShared());
		CreateQueryForUnit(UnitHandle, EntityQuery);
		EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
		FMassExecutionContext ExecutionContext(InEntityManager);

		TArray<FVector3f> RotatedNewPositions;
		TArray<FMassEntityHandle> Entities;
		
		{
			RTS::Stats::UpdateUnitPositionTimeSec = 0.0;
			FScopedDurationTimer DurationTimer(RTS::Stats::UpdateUnitPositionTimeSec);
			TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UpdateUnitPosition"))
			// Since get num matching entities does not return the correct value, we simply iterate through valid chunks to get # of entities in unit
			
			EntityQuery.ForEachEntityChunk(ExecutionContext, [&Entities](FMassExecutionContext& Context)
			{
				Entities.Append(Context.GetEntities());
			});
			
			// Calculate new positions for entities and output to NewPositions
			CalculateNewPositions(UnitFragment, Entities.Num(), NewPositions);

			// We need to rotate the positions to ensure we get accurate positions
			RotatedNewPositions = NewPositions;
			for (FVector3f& RotatedNewPosition : RotatedNewPositions)
			{
				RotatedNewPosition = RotatedNewPosition.RotateAngleAxis(UnitFragment.InterpRotation.Yaw, FVector3f(0.f,0.f,1.f));
				RotatedNewPosition += UnitFragment.InterpDestination;
			}
		}

		{
			// Apply new offsets to entities in unit
			RTS::Stats::UpdateEntityIndexTimeSec = 0.0;
			FScopedDurationTimer DurationTimer(RTS::Stats::UpdateEntityIndexTimeSec);
			EntityQuery.ForEachEntityChunk(ExecutionContext, [&NewPositions, &RotatedNewPositions](FMassExecutionContext& Context)
			{
				auto FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
				auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
			
				for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
				{
					FRTSFormationAgent& FormationAgent = FormationAgents[EntityIndex];
					auto& Transform = TransformFragments[EntityIndex].GetTransform();
			
					int ClosestIndex = 0;
					float DistSq = FLT_MAX;

					// Calculate distances
					auto Location = FVector3f(Transform.GetLocation());
					for (int i=0;i<NewPositions.Num();i++)
					{
						float Dist = FVector3f::DistSquared2D(RotatedNewPositions[i], Location);

						if (Dist < DistSq)
						{
							ClosestIndex = i;
							DistSq = Dist;
						}
					}
			
					FormationAgent.Offset = NewPositions[ClosestIndex];
				
					NewPositions.RemoveAtSwap(ClosestIndex, EAllowShrinking::No);
					RotatedNewPositions.RemoveAtSwap(ClosestIndex, EAllowShrinking::No);
				}
			});

			auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(InEntityManager.GetWorld());
			SignalSubsystem->SignalEntities(RTS::Unit::Signals::FormationUpdated, Entities);
		}
	});
}

void URTSFormationSubsystem::SetUnitPosition(const FVector& NewPosition, const FUnitHandle& UnitHandle)
{
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	EntityManager.ForEachSharedFragmentConditional<FUnitFragment>([&UnitHandle](FUnitFragment& UnitFragment)
	{
		return UnitHandle == UnitFragment.UnitHandle;
	},[&EntityManager, &NewPosition, &UnitHandle](FUnitFragment& UnitFragment)
	{
		auto NewPosition3f = FVector3f(NewPosition);
		DrawDebugDirectionalArrow(EntityManager.GetWorld(), NewPosition, FVector(NewPosition3f+((NewPosition3f-UnitFragment.InterpDestination).GetSafeNormal()*250.f)), 150.f, FColor::Red, false, 5.f, 0, 25.f);
	
		auto ForwardDir = (NewPosition3f-UnitFragment.InterpDestination).GetSafeNormal();
		UnitFragment.ForwardDir = FVector2f(ForwardDir.X, ForwardDir.Y);
		
		UnitFragment.UnitRotation = FRotator3f(UE::Math::TRotationMatrix<float>::MakeFromX(ForwardDir).Rotator());
		
		auto UnitInterpQuat = UnitFragment.InterpRotation.Quaternion();
		auto UnitQuat = UnitFragment.UnitRotation.Quaternion();
		
		bool bBlendAngle = FMath::RadiansToDegrees(UnitInterpQuat.AngularDistance(UnitQuat)) < 45;
		UnitFragment.InterpRotation = bBlendAngle ? UnitFragment.InterpRotation : UnitFragment.UnitRotation;
		
		{
			// Stop entity movement in unit
			FMassEntityQuery EntityQuery(EntityManager.AsShared());
			CreateQueryForUnit(UnitHandle, EntityQuery);
			EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);

			FMassExecutionContext Context(EntityManager);
			EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Context)
			{
				auto MoveTargetFragments = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

				for (auto Entity : Context.CreateEntityIterator())
				{
					MoveTargetFragments[Entity].CreateNewAction(EMassMovementAction::Stand, *Context.GetWorld());
				}
			});
		}
		
		UnitFragment.UnitDestination = NewPosition3f;

		if (!bBlendAngle)
		{
			FMassEntityQuery EntityQuery(EntityManager.AsShared());
			CreateQueryForUnit(UnitHandle, EntityQuery);
			
			EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
			FMassExecutionContext ExecContext(EntityManager);

			// Find the entity that is closest to the unit destination
			FVector ClosestLocation;
			float ClosestDistanceSq = FLT_MAX;
			EntityQuery.ForEachEntityChunk(ExecContext, [&NewPosition, &ClosestDistanceSq, &ClosestLocation](FMassExecutionContext& Context)
			{
				auto TransformFragments = Context.GetFragmentView<FTransformFragment>();
				for (int i=0;i<Context.GetNumEntities();i++)
				{
					const FVector& Location = TransformFragments[i].GetTransform().GetLocation();

					auto LocationDistanceSq = FVector::DistSquared2D(Location, NewPosition);
					if (LocationDistanceSq < ClosestDistanceSq)
					{
						ClosestDistanceSq = LocationDistanceSq;
						ClosestLocation = Location;
					}
				}
			});

			UnitFragment.InterpDestination = FVector3f(ClosestLocation);
		}
	});
	
	UpdateUnitPosition(UnitHandle);
}

void URTSFormationSubsystem::SpawnEntitiesForUnit(const FUnitHandle& UnitHandle, const UMassEntityConfigAsset* EntityConfig, int Count)
{
	if (!ensure(EntityConfig)) { return; }

	SpawnEntities(UnitHandle, EntityConfig->GetConfig(), Count);
}

void URTSFormationSubsystem::SpawnEntities(const FUnitHandle& UnitHandle, const FMassEntityConfig& EntityConfig,
	int Count)
{
	auto& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());

	// We are doing a little bit of work here since we are setting the unit index manually
	// Otherwise, using SpawnEntities would be perfectly fine

	auto& EntityTemplate = EntityConfig.GetOrCreateEntityTemplate(*GetWorld());
	EntityManager.Defer().PushCommand<FMassDeferredCreateCommand>([EntityTemplate, UnitHandle, Count](FMassEntityManager& InEntityManager)
	{
		FMassArchetypeSharedFragmentValues SharedFragmentValues = EntityTemplate.GetSharedFragmentValues();

		FUnitFragment UnitFragment = FUnitFragment();
		UnitFragment.UnitHandle = UnitHandle;

		auto& SharedUnitFragment = InEntityManager.GetOrCreateSharedFragment<FUnitFragment>(UnitFragment);
		SharedFragmentValues.Add(SharedUnitFragment);
		SharedFragmentValues.Sort();
		
		TArray<FMassEntityHandle> Entities;
		auto CreationContext = InEntityManager.BatchCreateEntities(EntityTemplate.GetArchetype(), SharedFragmentValues, Count, Entities);

		TConstArrayView<FInstancedStruct> FragmentInstances = EntityTemplate.GetInitialFragmentValues();
		InEntityManager.BatchSetEntityFragmentValues(CreationContext->GetEntityCollections(InEntityManager), FragmentInstances);
	});
}

FUnitHandle URTSFormationSubsystem::SpawnNewUnit(const UMassEntityConfigAsset* EntityConfig, int Count,
                                                 const FVector& Position)
{
	return SpawnUnit(EntityConfig->GetConfig(), Count, Position);
}

FUnitHandle URTSFormationSubsystem::SpawnUnit(const FMassEntityConfig& EntityConfig, int Count, const FVector& Position)
{
	auto UnitHandle = FUnitHandle();
	
	SpawnEntities(UnitHandle, EntityConfig, Count);
	return UnitHandle;
}

void URTSFormationSubsystem::SetFormationPreset(const FUnitHandle& UnitHandle, UFormationPresets* FormationAsset)
{
	if (!ensure(FormationAsset)) { return; }

	// @todo fix this
	/*
	UMassEntitySubsystem* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	auto& UnitSettings = EntityManager.GetSharedFragmentDataChecked<FUnitSettings>(UnitHandle);
	auto& UnitFragment = EntityManager.GetFragmentDataChecked<FUnitFragment>(UnitHandle);
	
	UnitSettings.FormationLength = FormationAsset->FormationLength;
	UnitSettings.BufferDistance = FormationAsset->BufferDistance;
	UnitSettings.Formation = FormationAsset->Formation;
	UnitSettings.Rings = FormationAsset->Rings;
	UnitSettings.bHollow = FormationAsset->bHollow;

	SetUnitPosition(UnitFragment.UnitPosition, UnitHandle);
	*/
}

void URTSFormationSubsystem::CalculateNewPositions(FUnitFragment& UnitFragment,
                                                   int Count, TArray<FVector3f>& OutNewPositions)
{
	// Empty NewPositions Map to make room for new calculations
	OutNewPositions.Empty(Count);
	auto& UnitSettings = UnitFragment.UnitSettings;
	
	// Calculate entity positions for new destination
	// This is the logic that can change formation types
	const FVector3f CenterOffset = FVector3f((Count/UnitSettings.FormationLength/2) * UnitSettings.BufferDistance, (UnitSettings.FormationLength/2) * UnitSettings.BufferDistance, 0.f);
	int PlacedUnits = 0;
	int PosIndex = 0;
	while (PlacedUnits < Count)
	{
		float w = PosIndex / UnitSettings.FormationLength;
		float l = PosIndex % UnitSettings.FormationLength;

		// Hollow formation logic (2 layers)
		if (UnitSettings.bHollow && UnitSettings.Formation == EFormationType::Rectangle)
		{
			int Switch = Count - UnitSettings.FormationLength*2;
			if (w != 0 && w != 1 && !(PlacedUnits >= Switch)
				&& l != 0 && l != 1 && l != UnitSettings.FormationLength-1 && l != UnitSettings.FormationLength-2)
			{
				PosIndex++;
				continue;
			}
		}

		// Circle formation
		if (UnitSettings.Formation == EFormationType::Circle)
		{
			int AmountPerRing = Count / UnitSettings.Rings;
			float Angle = PosIndex * PI * 2 / AmountPerRing;
			float Radius = UnitSettings.FormationLength + (PosIndex / AmountPerRing * 1.5f);
			w = FMath::Cos(Angle) * Radius;
			l = FMath::Sin(Angle) * Radius;
		}
		
		PlacedUnits++;
		FVector3f Position = FVector3f(w,l,0.f);
		Position *= UnitSettings.BufferDistance;
		if (UnitSettings.Formation == EFormationType::Rectangle)
		{
			FVector3f FrontOffset = CenterOffset;
			FrontOffset.X = 0.f;
			Position -= FrontOffset;
		}
			
		// Flip so that units are in correct spot
		Position = Position.RotateAngleAxis(180.f, FVector3f(0.f,0.f,1.f));
		
		OutNewPositions.Add(Position);
		//DrawDebugPoint(GetWorld(), Position+Unit.InterpolatedDestination, 20.f, FColor::Yellow, false, 10.f);
		PosIndex++;
	}
}

void URTSFormationSubsystem::CreateQueryForUnit(const FUnitHandle& UnitHandle, FMassEntityQuery& EntityQuery)
{
	EntityQuery.AddSharedRequirement<FUnitFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FRTSFormationAgent>(EMassFragmentAccess::ReadWrite);
	EntityQuery.SetChunkFilter([&UnitHandle](const FMassExecutionContext& Context)
	{
		auto& UnitFragment = Context.GetSharedFragment<FUnitFragment>();
		return UnitFragment.UnitHandle == UnitHandle;
	});
}
