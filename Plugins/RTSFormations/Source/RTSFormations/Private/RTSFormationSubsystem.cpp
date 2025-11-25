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

	// In order for entities to move to the optimal spot, updating the unit position has to do a few calculations
	// 1. Calculate new positions for every single unit
	// 2. Prepare for distance calculations by applying rotation offsets
	// 3. Add to hash grid for efficient queries
	// 4. Iterate through all entities in the unit and find the nearest point - this is first done by iteratively querying the hash grid, then doing a distance check
	// 5. Apply offset and remove from grid to avoid multiple entities claiming the same spot
	// 6. Signal all entities in unit that they can now start moving to their desired spot

	struct FMortonItem
	{
		FMortonItem(uint32 Morton, const FVector3f& Pos) : Position(Pos), MortonValue(Morton) {};

		FVector3f Position;
		uint32 MortonValue;
		bool bClaimed = false;

		bool operator<(const FMortonItem& Other) const
		{
			return MortonValue < Other.MortonValue;
		}
	};
	
	auto& InEntityManager = UE::Mass::Utils::GetEntityManagerChecked(*GetWorld());
	TRACE_CPUPROFILER_EVENT_SCOPE(UpdateUnitPosition)
	
	InEntityManager.ForEachSharedFragmentConditional<FUnitFragment>([&UnitHandle](const FUnitFragment& InUnitFragment)
	{
		return InUnitFragment.UnitHandle == UnitHandle;
	},
	[&InEntityManager, &UnitHandle](FUnitFragment& UnitFragment)
	{
		auto GridItemExtent = FVector(5.f);
		int CellSize = 50.f;
		int Offset = 100000;
		
		FMassEntityQuery EntityQuery(InEntityManager.AsShared());
		CreateQueryForUnit(UnitHandle, EntityQuery);
		FMassExecutionContext ExecutionContext(InEntityManager);
		
		TArray<FMassEntityHandle> Entities;
		TArray<FVector3f> NewPositions;
		TArray<FMortonItem> MortonArray;
		
		auto& TargetRotation = UnitFragment.bSnapToUnitRotation ? UnitFragment.UnitRotation.Yaw : UnitFragment.InterpRotation.Yaw;
		
		{
			RTS::Stats::UpdateUnitPositionTimeSec = 0.0;
			TRACE_CPUPROFILER_EVENT_SCOPE(GenerateUnitPositions)
			FScopedDurationTimer DurationTimer(RTS::Stats::UpdateUnitPositionTimeSec);
			// Since get num matching entities does not return the correct value, we simply iterate through valid chunks to get # of entities in unit
			
			EntityQuery.ForEachEntityChunk(ExecutionContext, [&Entities](FMassExecutionContext& Context)
			{
				Entities.Append(Context.GetEntities());
			});
			
			// Calculate new positions for entities and output to NewPositions
			CalculateNewPositions(UnitFragment, Entities.Num(), NewPositions);

			// Perform rotation so distance queries are correct - we will undo this after
			auto RotationAxis = FVector3f(0.f,0.f,1.f);
			for (FVector3f& Position : NewPositions)
			{
				Position = Position.RotateAngleAxis(TargetRotation, RotationAxis);
			}

			//@todo testing morton
			MortonArray.Reserve(NewPositions.Num());
			
			for (const FVector3f& NewPosition : NewPositions)
			{
				FIntPoint Point(NewPosition.X / CellSize + Offset, NewPosition.Y / CellSize + Offset);
				uint32 Morton = FMath::MortonCode2(Point.X) | (FMath::MortonCode2(Point.Y) << 1);
				MortonArray.Add(FMortonItem(Morton, NewPosition));
			}

			MortonArray.Sort();
		}

		{
			// Apply new offsets to entities in unit
			RTS::Stats::UpdateEntityIndexTimeSec = 0.0;
			FScopedDurationTimer DurationTimer(RTS::Stats::UpdateEntityIndexTimeSec);
			TRACE_CPUPROFILER_EVENT_SCOPE(UpdateEntityIndex)

			FVector Extent = FVector(50.f);
			EntityQuery.ForEachEntityChunk(ExecutionContext, [&MortonArray, &UnitFragment, &GridItemExtent, &TargetRotation, &Extent, &CellSize, &Offset](FMassExecutionContext& Context)
			{
				auto FormationAgents = Context.GetMutableFragmentView<FRTSFormationAgent>();
			
				for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
				{
					FRTSFormationAgent& FormationAgent = FormationAgents[EntityIndex];
			
					int ClosestIndex = INDEX_NONE;
					float DistSq = FLT_MAX;

					// Apply rotation offset to agent offset for accurate distance check
					auto RotatedAgentOffset = FormationAgent.Offset.RotateAngleAxis(UnitFragment.InterpRotation.Yaw, FVector3f(0.f,0.f,1.f));

					FIntPoint Point(RotatedAgentOffset.X / CellSize + Offset, RotatedAgentOffset.Y / CellSize + Offset);
					uint32 Morton = FMath::MortonCode2(Point.X) | (FMath::MortonCode2(Point.Y) << 1);
					int32 ClosestIdx = Algo::LowerBoundBy(MortonArray, Morton, [](const FMortonItem& E) { return E.MortonValue; });
					ClosestIdx = FMath::Clamp(ClosestIdx, 0, MortonArray.Num() - 1);

					int Left = INDEX_NONE;
					int Right = INDEX_NONE;

					// Right side
					for (int i=ClosestIdx;i<MortonArray.Num();i++)
					{
						if (!MortonArray[i].bClaimed)
						{
							Right = i;
							break;
						}
					}

					// Left side
					for (int i=ClosestIdx;i>=0;i--)
					{
						if (!MortonArray[i].bClaimed)
						{
							Left = i;
							break;
						}
					}

					float LeftDist = MortonArray.IsValidIndex(Left) ? FVector3f::DistSquared2D(MortonArray[Left].Position, RotatedAgentOffset) : FLT_MAX;
					float RightDist = MortonArray.IsValidIndex(Right) ? FVector3f::DistSquared2D(MortonArray[Right].Position, RotatedAgentOffset) : FLT_MAX;
					ClosestIndex = LeftDist < RightDist ? Left : Right;
					MortonArray[ClosestIndex].bClaimed = true;

					// Undo rotation and apply offset
					FormationAgent.Offset = MortonArray[ClosestIndex].Position.RotateAngleAxis(TargetRotation, FVector3f(0.f,0.f,-1.f));
				}
			});
		}
		
		auto SignalSubsystem = UWorld::GetSubsystem<UMassSignalSubsystem>(InEntityManager.GetWorld());
		SignalSubsystem->SignalEntities(RTS::Unit::Signals::FormationUpdated, Entities);
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
		
		UnitFragment.bSnapToUnitRotation = FMath::RadiansToDegrees(UnitInterpQuat.AngularDistance(UnitQuat)) > 45;
		
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

		if (UnitFragment.bSnapToUnitRotation)
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
